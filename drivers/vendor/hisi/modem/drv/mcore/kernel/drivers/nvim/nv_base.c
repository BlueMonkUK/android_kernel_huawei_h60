

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*lint -save -e537*/
#include <bsp_nvim.h>
#include <bsp_hardtimer.h>
#include "nv_comm.h"
/*lint -restore +e537*/



u32 nv_readEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret = NV_ERROR;
    struct nv_file_list_info_stru file_info = {0};
    struct nv_ref_data_info_stru  ref_info  = {0};

    if((NULL == pdata)||(0 == datalen))
    {
        ret = BSP_ERR_NV_INVALID_PARAM;
        goto nv_readEx_err;
    }

    ret = nv_search_byid(itemid,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info,&file_info);
    if(ret)
    {
        goto nv_readEx_err;
    }

    if((offset + datalen) > ref_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        goto nv_readEx_err;
    }
    if(modem_id <= ref_info.modem_num)
    {
        ref_info.nv_off += (modem_id - NV_USIMM_CARD_1)*ref_info.nv_len;
    }
    else
    {
        ret = BSP_ERR_NV_INVALID_PARAM;
        goto nv_readEx_err;
    }

    ret = nv_read_from_mem(pdata, datalen,file_info.file_id,ref_info.nv_off+offset);
    if(ret)
    {
        goto nv_readEx_err;
    }

    return NV_OK;
nv_readEx_err:
    return ret;
}
#ifndef BSP_CONFIG_HI3630
u32 nv_writeEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret = NV_ERROR;
    struct nv_file_list_info_stru file_info = {0};
    struct nv_ref_data_info_stru  ref_info  = {0};

    if((NULL == pdata)||(0 == datalen))
    {
        ret = BSP_ERR_NV_INVALID_PARAM;
        goto nv_writeEx_err;
    }

    ret = nv_search_byid(itemid,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info,&file_info);
    if(ret)
    {

        goto nv_writeEx_err;
    }

    if((datalen + offset) >ref_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        goto nv_writeEx_err;
    }
    if(modem_id <= ref_info.modem_num)
    {
        ref_info.nv_off += (modem_id - NV_USIMM_CARD_1)*ref_info.nv_len;
    }
    else
    {

        ret = BSP_ERR_NV_INVALID_PARAM;
        goto nv_writeEx_err;
    }

    ret = nv_write_to_mem(pdata,datalen,file_info.file_id,ref_info.nv_off+offset);
    if(ret)
    {
        goto nv_writeEx_err;
    }

    ret = nv_write_to_file(&ref_info);
    if(ret)
    {
        goto nv_writeEx_err;
    }
    return NV_OK;
nv_writeEx_err:
    return ret;
}
#endif

u32 bsp_nvm_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}
u32 bsp_nvm_dcreadpart(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,offset,pdata,datalen);
}

#ifndef BSP_CONFIG_HI3630
u32 bsp_nvm_dcwritepart(u32 modem_id,u32 itemid, u32 offset,u8* pdata,u32 datalen)
{
    if(false == nv_write_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,offset,pdata,datalen);
}





u32 bsp_nvm_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    if(false == nv_write_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}


u32 bsp_nvm_flushEx(u32 off,u32 len,u32 itemid)
{
    u32 ret = NV_ERROR;
    struct nv_icc_stru icc_req = {0};
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    if(NV_INIT_OK != ddr_info->mcore_init_state)
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    icc_req.msg_type = NV_ICC_REQ;
    icc_req.data_len = len;
    icc_req.data_off = off;
    icc_req.ret      = 93;   /*use to test ,no meaning*/
    icc_req.itemid   = itemid;
    icc_req.slice    = bsp_get_slice_value();

    ret = nv_icc_send((u8*)&icc_req,sizeof(icc_req));

    return ret;
}
u32 bsp_nvm_flush(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;

    return bsp_nvm_flushEx(ctrl_info->ctrl_size,(ddr_info->file_len-ctrl_info->ctrl_size),NV_ERROR);
}



u32 bsp_nvm_flushSys(u32 itemid)
{
    u32 ret = NV_ERROR;
    struct nv_icc_stru icc_req = {0};
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;


    if(NV_INIT_OK != ddr_info->ccore_init_state)
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    icc_req.msg_type = NV_ICC_REQ_SYS;
    icc_req.data_len = ddr_info->file_len;
    icc_req.data_off = 0;
    icc_req.ret      = 93;   /*use to test ,no meaning*/
    icc_req.itemid   = itemid;
    icc_req.slice    = bsp_get_slice_value();

    ret = nv_icc_send((u8*)&icc_req,sizeof(icc_req));

    return ret;
}
#endif
u32 bsp_nvm_init(void)
{

    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    if(ddr_info->mcore_init_state < NV_BOOT_INIT_OK)
    {
        ret = BSP_ERR_NV_MEM_INIT_FAIL;
        goto nv_init_fail;
    }

    ret = nv_icc_chan_init();
    if(ret)
    {
        goto nv_init_fail;
    }
    g_nv_ctrl.mid_prio = 20;
    ddr_info->mcore_init_state = NV_INIT_OK;

    return NV_OK;
nv_init_fail:
    printf("nv init err:0x%x\n",ret);
    return ret;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif




