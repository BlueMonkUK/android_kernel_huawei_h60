


/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include  "NasERabmEsmMsgProc.h"
#include  "NasERabmAppMsgProc.h"
#include  "NasERabmRrcMsgProc.h"
#include  "NasERabmIpFilter.h"
#include  "NasERabmCdsMsgProc.h"

/*lint -e767*/
#define    THIS_FILE_ID        PS_FILE_ID_NASRABMESMMSGPROC_C
/*lint +e767*/

/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/


/*****************************************************************************
  3 Function
*****************************************************************************/


/*lint -e960*/
/*lint -e961*/
VOS_VOID NAS_ERABM_EsmMsgDistr( VOS_VOID *pRcvMsg )
{
    PS_MSG_HEADER_STRU                 *pSmMsg;

    pSmMsg = (PS_MSG_HEADER_STRU*)pRcvMsg;

    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_EsmMsgDistr is entered.");

    /*根据消息名，调用相应的消息处理函数*/
    switch (pSmMsg->ulMsgName)
    {
        /*如果收到的是ESM_ERABM_ACT_IND消息，调用NAS_ERABM_RcvRabmEsmActInd函数*/
        case ID_ESM_ERABM_ACT_IND:
            NAS_ERABM_RcvRabmEsmActInd((ESM_ERABM_ACT_IND_STRU *) pRcvMsg);
            break;

        /*如果收到的是ESM_ERABM_MDF_IND消息，调用NAS_ERABM_RcvRabmEsmMdfInd函数*/
        case ID_ESM_ERABM_MDF_IND:
            NAS_ERABM_RcvRabmEsmMdfInd((ESM_ERABM_MDF_IND_STRU *) pRcvMsg);
            break;

        /*如果收到的是ESM_ERABM_DEACT_IND消息，调用NAS_ERABM_RcvRabmEsmDeactInd函数*/
        case ID_ESM_ERABM_DEACT_IND:
            NAS_ERABM_RcvRabmEsmDeactInd((ESM_ERABM_DEACT_IND_STRU *) pRcvMsg);
            break;

        /*如果收到的是ESM_ERABM_REL_IND消息，调用NAS_ERABM_RcvRabmEsmRelInd函数*/
        case ID_ESM_ERABM_REL_IND:
            NAS_ERABM_RcvRabmEsmRelInd();
            break;

        default:
            NAS_ERABM_WARN_LOG("NAS_ERABM_EsmMsgDistr:WARNING:SM->RABM Message name non-existent!");
            break;
    }

    /*遍历承载激活表，如果只有一条承载激活，判断该承载如果没有上行TFT,记录该承载号*/
    NAS_ERABM_SrchOnlyActiveAndNoUlTftEpsb();

    return;
}
VOS_VOID  NAS_ERABM_SrchOnlyActiveAndNoUlTftEpsb( VOS_VOID )
{

    VOS_UINT32                          ulEpsbId = NAS_ERABM_NULL;
    VOS_UINT32                          ulTempActEpsbId = NAS_ERABM_NULL;
    VOS_UINT32                          ulActEpsbNum = NAS_ERABM_NULL;

    /*遍历承载激活表，记录激活承载的个数和最后一条激活承载号*/
    for (ulEpsbId = NAS_ERABM_MIN_EPSB_ID; ulEpsbId <= NAS_ERABM_MAX_EPSB_ID; ulEpsbId++)
    {
        if (NAS_ERABM_EPSB_ACTIVE == NAS_ERABM_GetEpsbStateInfo(ulEpsbId))
        {
            ulTempActEpsbId = ulEpsbId;
            ulActEpsbNum++;
        }
    }

    /*如果激活承载数不是 1，则不记录任何承载，返回*/
    if(NAS_ERABM_ACT_EPSB_NUM_ONE != ulActEpsbNum)
    {
        NAS_ERABM_SetOnlyActiveAndNoUlTftEpsbId(NAS_ERABM_ILL_EPSB_ID);
        NAS_ERABM_INFO_LOG("NAS_ERABM_SrchOnlyActiveAndNoUlTftEpsb:the active EpsbNum is not one!");
        return;
    }

    /*如果只有一条激活承载，判断该承载如果不存在上行TFT，记录承载号*/
    if(PS_FALSE == NAS_ERABM_IsExistUplinkPfinTft(ulTempActEpsbId))
    {
        NAS_ERABM_SetOnlyActiveAndNoUlTftEpsbId(ulTempActEpsbId);
    }
    else
    {
        NAS_ERABM_SetOnlyActiveAndNoUlTftEpsbId(NAS_ERABM_ILL_EPSB_ID);
    }

    return;
}
VOS_VOID NAS_ERABM_SaveUeIpAddr(const ESM_ERABM_ACT_IND_STRU *pRcvMsg, VOS_UINT32 ulEpsbId)
{
    VOS_UINT8                           ucIpType   = ESM_ERABM_PDN_TYPE_IPV4;
    APP_ESM_IP_ADDR_STRU               *pstPdnAddr = VOS_NULL_PTR;

    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_SaveUeIpAddr is entered.");

    ucIpType = pRcvMsg->stPdnAddr.ucIpType;

    pstPdnAddr = NAS_ERABM_GetEpsbPdnAddr(ulEpsbId);

    switch(ucIpType)
    {
        /*保存IPv4地址*/
        case ESM_ERABM_PDN_TYPE_IPV4:
            pstPdnAddr->ucIpType = ESM_ERABM_PDN_TYPE_IPV4;
            NAS_ERABM_MEM_CPY(pstPdnAddr->aucIpV4Addr, pRcvMsg->stPdnAddr.aucIpV4Addr,\
                                                              APP_MAX_IPV4_ADDR_LEN);
            break;

        /*保存IPv6地址*/
        case ESM_ERABM_PDN_TYPE_IPV6:
            pstPdnAddr->ucIpType = ESM_ERABM_PDN_TYPE_IPV6;
            NAS_ERABM_MEM_CPY(pstPdnAddr->aucIpV6Addr, pRcvMsg->stPdnAddr.aucIpV6Addr,\
                                                              APP_MAX_IPV6_ADDR_LEN);
            break;

        /*依次存放IPV4和IPV6地址*/
        case ESM_ERABM_PDN_TYPE_IPV4_IPV6:
            pstPdnAddr->ucIpType = ESM_ERABM_PDN_TYPE_IPV4_IPV6;
            NAS_ERABM_MEM_CPY(pstPdnAddr->aucIpV4Addr, pRcvMsg->stPdnAddr.aucIpV4Addr,\
                                                              APP_MAX_IPV4_ADDR_LEN);
            NAS_ERABM_MEM_CPY(pstPdnAddr->aucIpV6Addr, pRcvMsg->stPdnAddr.aucIpV6Addr,\
                                                              APP_MAX_IPV6_ADDR_LEN);
            break;

        default:
            NAS_ERABM_WARN_LOG("NAS_ERABM_SaveUeIpAddr:WARNING:ESM->RABM Invalid IP Type!");
            NAS_ERABM_MEM_SET((VOS_VOID*)pstPdnAddr,
                        0,
                        sizeof(APP_ESM_IP_ADDR_STRU));
            break;
    }
}


VOS_VOID NAS_ERABM_SaveEpsbActInfo(const ESM_ERABM_ACT_IND_STRU *pRcvMsg, VOS_UINT32 ulEpsbId )
{
    VOS_UINT32                          ulEpsTftPfNum           = pRcvMsg->ulTftPfNum;
    VOS_UINT32                          ulTftPfCnt              = NAS_ERABM_NULL;
    ESM_ERABM_TFT_PF_STRU               *pstTftPf                = VOS_NULL_PTR;

    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_SaveEpsbActInfo is entered.");

    if(ulEpsTftPfNum > ESM_ERABM_MAX_PF_NUM_IN_BEARER)
    {
        ulEpsTftPfNum = ESM_ERABM_MAX_PF_NUM_IN_BEARER;
    }

    /*保存IP地址*/
    NAS_ERABM_SaveUeIpAddr(pRcvMsg, ulEpsbId);

    /* 记录承载所包含的所有PF信息 */
    for(ulTftPfCnt = 0; ulTftPfCnt < ulEpsTftPfNum; ulTftPfCnt++)
    {
        pstTftPf = NAS_ERABM_GetEpsbTftAddr(ulEpsbId, ulTftPfCnt);

        /* 设置新PF信息 */
        NAS_ERABM_MEM_CPY(pstTftPf, &pRcvMsg->astTftPf[ulTftPfCnt], sizeof(ESM_ERABM_TFT_PF_STRU));

        /* 为了防止NDIS所需数据包误发送到空口，NDIS所需数据包的过滤器的优先级
           要高于空口消息中的TFT的优先级，目前优先级0-5预留给NDIS所需数据包的
           过滤器优先级 */
        pstTftPf->ucPrecedence += NAS_ERABM_IPF_UL_NDIS_PRIORITY_NUM;
    }

    /* 设置承载包含的PF数量 */
    NAS_ERABM_SetEpsbTftPfNum(ulEpsbId, ulEpsTftPfNum);

    /*记录承载上下文激活状态*/
    NAS_ERABM_SetEpsbStateInfo(ulEpsbId, NAS_ERABM_EPSB_ACTIVE);

    NAS_ERABM_SetEpsbBearerType(ulEpsbId, (pRcvMsg->enBearerCntxtType));
    NAS_ERABM_SetEpsbLinkedEpsbId(ulEpsbId, (pRcvMsg->ulLinkedEpsbId));
    NAS_ERABM_SetEpsbQCI(ulEpsbId, (pRcvMsg->ucQCI));

    /* 如果第一个承载激活且不是环回模式时，设置enUlNdisFilterValidFlag为有效 */
    if ((1 == NAS_ERABM_GetActiveEpsBearerNum())
        && (NAS_ERABM_MODE_TYPE_NORMAL == NAS_ERABM_GetMode()))
    {
        NAS_ERABM_SetUlNdisFilterValidFlag(NAS_ERABM_UL_NDIS_FILTER_VALID);
    }
}
VOS_VOID NAS_ERABM_ClearEpsbResource( VOS_UINT32 ulEpsbId )
{
    VOS_UINT32                          ulEpsTftPfNum   = NAS_ERABM_NULL;
    VOS_UINT32                          ulTftPfCnt      = NAS_ERABM_NULL;
    ESM_ERABM_TFT_PF_STRU               *pstTftPf        = VOS_NULL_PTR;
    APP_ESM_IP_ADDR_STRU               *pstPdnAddr      = VOS_NULL_PTR;

    NAS_ERABM_SetEpsbStateInfo(ulEpsbId, NAS_ERABM_EPSB_INACTIVE);

    pstPdnAddr = NAS_ERABM_GetEpsbPdnAddr(ulEpsbId);

    ulEpsTftPfNum = NAS_ERABM_GetEpsbTftPfNum(ulEpsbId);

    /* 清空承载包含的所有PF信息和PF标志 */
    for(ulTftPfCnt = 0; ulTftPfCnt < ulEpsTftPfNum; ulTftPfCnt++)
    {
         pstTftPf = NAS_ERABM_GetEpsbTftAddr(ulEpsbId, ulTftPfCnt);

        /* 清空PF信息 */
        NAS_ERABM_MEM_SET(pstTftPf, NAS_ERABM_NULL, sizeof(ESM_ERABM_TFT_PF_STRU));
    }

    NAS_ERABM_SetEpsbTftPfNum(ulEpsbId, NAS_ERABM_NULL);

    NAS_ERABM_MEM_SET(pstPdnAddr, NAS_ERABM_NULL, sizeof(APP_ESM_IP_ADDR_STRU));
}



VOS_VOID  NAS_ERABM_RcvRabmEsmActInd(const ESM_ERABM_ACT_IND_STRU *pRcvMsg )
{
    VOS_UINT32               ulEpsbId          = 0;                         /*用来记录EPS承载激活表中的EPSID*/

    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_RcvRabmEsmActInd is entered.");

    /*找到对应的EPS承载*/
    ulEpsbId = pRcvMsg->ulEpsId;

    /*清除对应的EPS承载的相关信息*/
    NAS_ERABM_ClearEpsbResource(ulEpsbId);

    /*记录相应的参数*/
    NAS_ERABM_SaveEpsbActInfo(pRcvMsg, ulEpsbId);

    /* 给CDS发送CDS_ERABM_RAB_CREATE_IND */
    NAS_ERABM_SndErabmCdsRabCreatInd(ulEpsbId);

    /*判断是否在等待承载激活*/
    if (NAS_ERABM_WAIT_EPSB_ACT_MSG == NAS_ERABM_GetWaitEpsbActSign())
    {
        /* 判断是否存在DRB已建立，但关联的EPS承载尚未激活的情况*/
        if (NAS_ERABM_FAILURE == NAS_ERABM_HasDrbWithoutRelatingEpsBear())
        {
            /*停止建立等待定时器*/
            NAS_ERABM_TimerStop(NAS_ERABM_WAIT_EPSB_ACT_TIMER);

            /* 向ESM发承载状态消息 */
            NAS_ERABM_InformEsmBearerStatus();

            /*清除等待标识*/
            NAS_ERABM_SetWaitEpsBActSign(NAS_ERABM_NOT_WAIT_EPSB_ACT_MSG);
        }
    }

    /* 挂起态下不需要配置IPF */
    if (NAS_ERABM_L_MODE_STATUS_NORMAL != NAS_ERABM_GetLModeStatus())
    {
        return ;
    }
    /*根据TFT和承载信息更新IP Filter到硬件加速器*/
    if(PS_FAIL == NAS_ERABM_IpfConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: IPF Update IP Filter Failed.");
    }

    /*根据TFT和承载信息更新IP Filter到CDS*/
    if(PS_FAIL == NAS_ERABM_CdsConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: CDS Update IP Filter Failed.");
    }
}

/*****************************************************************************
 Function Name   : NAS_ERABM_RcvRabmEsmMdfInd()
 Description     : RABM模块ESM_ERABM_MDF_IND_STRU消息分发处理
 Input           : ESM_ERABM_MDF_IND_STRU *pRcvMsg-----------消息指针
 Output          : VOS_VOID
 Return          : VOS_VOID

 History         :
    1.RabmStru      2008-9-5  Draft Enact

*****************************************************************************/
VOS_VOID NAS_ERABM_RcvRabmEsmMdfInd(const ESM_ERABM_MDF_IND_STRU *pRcvMsg )
{
    VOS_UINT32                         ulEpsbId = 0;

    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_RcvRabmEsmMdfInd is entered.");

    /*找到要修改的EPS承载*/
    ulEpsbId = pRcvMsg->ulEpsId;
    /* 防止在保存承载相应参数过程中，上层灌包，CDS切走任务，调用接口获取承载号由于承载信息还没有保存完导致获取承载号失败，所以在此处锁任务  */
    (VOS_VOID)VOS_TaskLock();

    /*清除对应的EPS承载的相关信息*/
    NAS_ERABM_ClearEpsbResource(ulEpsbId);

    /*记录相应的参数*/
    NAS_ERABM_SaveEpsbActInfo(pRcvMsg, ulEpsbId);

    (VOS_VOID)VOS_TaskUnlock();

    /* 给CDS发送CDS_ERABM_RAB_CREATE_IND_STRU */
    NAS_ERABM_SndErabmCdsRabCreatInd(ulEpsbId);

    /* 挂起态下不需要配置IPF */
    if (NAS_ERABM_L_MODE_STATUS_NORMAL != NAS_ERABM_GetLModeStatus())
    {
        return ;
    }

    /*根据TFT和承载信息更新IP Filter到硬件加速器*/
    if(PS_FAIL == NAS_ERABM_IpfConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: IPF Update IP Filter Failed.");
    }

    /*根据TFT和承载信息更新IP Filter到CDS*/
    if(PS_FAIL == NAS_ERABM_CdsConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: CDS Update IP Filter Failed.");
    }
}


/*****************************************************************************
 Function Name   : NAS_ERABM_RcvRabmEsmDeactInd()
 Description     : RABM模块ESM_ERABM_DEACT_IND_STRU消息分发处理
 Input           : ESM_ERABM_DEACT_IND_STRU *pRcvMsg-----------消息指针
 Output          : VOS_VOID
 Return          : VOS_VOID

 History         :
    1.yangqianhui      2008-9-5  Draft Enact

*****************************************************************************/
VOS_VOID NAS_ERABM_RcvRabmEsmDeactInd(const ESM_ERABM_DEACT_IND_STRU *pRcvMsg )
{
    VOS_UINT32          ulEpsbId   = 0;
    VOS_UINT32          ulEpsIdCnt = 0;
    VOS_UINT32          ulLoop     = 0;


    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_RcvRabmEsmDeactInd is entered.");

    /*获取去激活的承载数*/
    ulEpsIdCnt = pRcvMsg->ulEpsIdNum;

    /*找到要去激活的EPS承载*/
    for (ulLoop = 0; ulLoop < ulEpsIdCnt; ulLoop++)
    {
        ulEpsbId = pRcvMsg->aulEpsId[ulLoop];

        /* 给CDS发送CDS_ERABM_RAB_RELEASE_IND_STRU */
        NAS_ERABM_SndErabmCdsRabReleaseInd(ulEpsbId);

        /*RABM释放与其相关的资源*/
        NAS_ERABM_ClearEpsbResource(ulEpsbId);
        NAS_ERABM_SetRbStateInfo(ulEpsbId, NAS_ERABM_RB_DISCONNECTED);
        NAS_ERABM_SetEpsbRbIdInfo(ulEpsbId, NAS_ERABM_ILL_RB_ID);
    }

    /* 如果承载被释放，设置enUlNdisFilterValidFlag为无效 */
    if (0 == NAS_ERABM_GetActiveEpsBearerNum())
    {
        NAS_ERABM_SetUlNdisFilterValidFlag(NAS_ERABM_UL_NDIS_FILTER_INVALID);

        /* lihong00150010 erabm code begin */
        /* 通知CDS释放缓存的数据包 */
        NAS_ERABM_SndErabmCdsFreeBuffDataInd();
        /* lihong00150010 erabm code end */
    }

    /* 挂起态下不需要配置IPF */
    if (NAS_ERABM_L_MODE_STATUS_NORMAL != NAS_ERABM_GetLModeStatus())
    {
        return ;
    }

    /*根据TFT和承载信息更新IP Filter到硬件加速器*/
    if(PS_FAIL == NAS_ERABM_IpfConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: IPF Update IP Filter Failed.");
    }

    /*根据TFT和承载信息更新IP Filter到CDS*/
    if(PS_FAIL == NAS_ERABM_CdsConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: CDS Update IP Filter Failed.");
    }
}

/*****************************************************************************
 Function Name   : NAS_ERABM_RcvRabmEsmRelInd()
 Description     : RABM模块ESM_ERABM_REL_IND_STRU消息分发处理
 Input           : VOS_VOID
 Output          : VOS_VOID
 Return          : VOS_VOID

 History         :
    1.RabmStru      2008-9-5  Draft Enact

*****************************************************************************/
VOS_VOID NAS_ERABM_RcvRabmEsmRelInd( VOS_VOID )
{
    VOS_UINT32 ulLoop = NAS_ERABM_NULL;


    /*打印进入该函数*/
    NAS_ERABM_INFO_LOG("NAS_ERABM_RcvRabmEsmRelInd is entered.");

    /* 通知CDS RAB RELEASE */
    for (ulLoop = NAS_ERABM_MIN_EPSB_ID; ulLoop <= NAS_ERABM_MAX_EPSB_ID; ulLoop++)
    {
        if (NAS_ERABM_EPSB_ACTIVE == NAS_ERABM_GetEpsbStateInfo(ulLoop))
        {
            /* 给CDS发送CDS_ERABM_RAB_RELEASE_IND */
            NAS_ERABM_SndErabmCdsRabReleaseInd(ulLoop);
        }
    }

    /*释放所有资源，回到开机初始化后的状态*/
    NAS_ERABM_ClearRabmResource();

    /*根据TFT和承载信息更新IP Filter到硬件加速器*/
    if(PS_FAIL == NAS_ERABM_IpfConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: IPF Update IP Filter Failed.");
    }

    /*根据TFT和承载信息更新IP Filter到CDS*/
    if(PS_FAIL == NAS_ERABM_CdsConfigUlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_EsmMsgDistr: CDS Update IP Filter Failed.");
    }
    #if 0
    /* 配置下行TFT信息 */
    if(PS_FAIL == NAS_ERABM_IpfConfigDlFilter())
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_RcvRabmEmmResumeInd: Update DL IP Filter Failed.");
    }
    #endif
}

VOS_VOID NAS_ERABM_SndRabmEsmRelReq( VOS_VOID )
{
    ESM_ERABM_REL_REQ_STRU    *pstRelReq = VOS_NULL_PTR;

    /*分配空间和检测是否分配成功*/
    pstRelReq = (VOS_VOID*)NAS_ERABM_ALLOC_MSG(sizeof(ESM_ERABM_REL_REQ_STRU));
    if (VOS_NULL_PTR == pstRelReq)
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_SndRabmEsmRelReq:ERROR:Alloc msg fail!" );
        return;
    }

    /*清空*/
    NAS_ERABM_MEM_SET(NAS_ERABM_GET_MSG_ENTITY(pstRelReq), NAS_ERABM_NULL, NAS_ERABM_GET_MSG_LENGTH(pstRelReq));

    /*填写消息头*/
    NAS_ERABM_WRITE_ESM_MSG_HEAD(pstRelReq,ID_ESM_ERABM_REL_REQ);

    /* 调用消息发送函数*/
    NAS_ERABM_SND_MSG(pstRelReq);

}

/*****************************************************************************
 Function Name   : NAS_ERABM_SndRabmEsmBearerStatusReq
 Description     : 向ESM发送建立rb的EpsbId信息
 Input           : VOS_UINT32 *pulEpsbId
                   VOS_UINT32 ulEpsbIdNum
 Output          : None
 Return          : VOS_VOID

 History         :
    1.sunbing49683      2009-3-28  Draft Enact

*****************************************************************************/
VOS_VOID NAS_ERABM_SndRabmEsmBearerStatusReq(const VOS_UINT32 *pulEpsbId,
                                                              VOS_UINT32 ulEpsbIdNum )
{
    ESM_ERABM_BEARER_STATUS_REQ_STRU    *pstBearerStatusReq  = VOS_NULL_PTR;

    /*入口参数检查*/
    if ((VOS_NULL_PTR == pulEpsbId) || (ulEpsbIdNum > NAS_ERABM_MAX_EPSB_NUM))
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_SndRabmEsmBearerStatusReq:ERROR:Input para error!");
        return;
    }

    /*分配空间和检测是否分配成功*/
    pstBearerStatusReq = (VOS_VOID*)NAS_ERABM_ALLOC_MSG(sizeof(ESM_ERABM_BEARER_STATUS_REQ_STRU));
    if ( VOS_NULL_PTR == pstBearerStatusReq )
    {
        NAS_ERABM_ERR_LOG("NAS_ERABM_SndRabmEsmBearerStatusReq:ERROR:Alloc msg fail!" );
        return;
    }

    NAS_ERABM_MEM_SET(NAS_ERABM_GET_MSG_ENTITY(pstBearerStatusReq), NAS_ERABM_NULL,\
                     NAS_ERABM_GET_MSG_LENGTH(pstBearerStatusReq));

    /*填写消息头*/
    NAS_ERABM_WRITE_ESM_MSG_HEAD(pstBearerStatusReq, ID_ESM_ERABM_BEARER_STATUS_REQ);

    /*填写消息实体*/
    pstBearerStatusReq->ulEpsIdNum = ulEpsbIdNum;
    NAS_ERABM_MEM_CPY((VOS_UINT8*)pstBearerStatusReq->aulEpsId, (VOS_UINT8*)pulEpsbId,
              (pstBearerStatusReq->ulEpsIdNum)*(sizeof(VOS_UINT32)/sizeof(VOS_UINT8)));

    /*调用消息发送函数*/
    NAS_ERABM_SND_MSG(pstBearerStatusReq);

}
/*lint +e961*/
/*lint +e960*/




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

