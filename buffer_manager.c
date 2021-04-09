#include "buffer_manager.h"
#include <string.h>

/*push one item per operation*/
int32_t push_circle_buff_item(buffer_info_t *circle_buff_info, void *push_ptr, int32_t length)
{
    int32_t delta_offset = 0;

    int32_t ret_val = 0;

    delta_offset = circle_buff_info->tail_index_offset
                            -circle_buff_info->head_index_offset;
    printf("head:%d tail:%d\n",circle_buff_info->head_index_offset, circle_buff_info->tail_index_offset);
    if(delta_offset>=0)
    {
        if((circle_buff_info->tail_index_offset+length)>=circle_buff_info->buff_length)
        {
            memcpy(circle_buff_info->buff+circle_buff_info->tail_index_offset,push_ptr
                    ,(circle_buff_info->buff_length-circle_buff_info->tail_index_offset));

            memcpy(circle_buff_info->buff,push_ptr+(circle_buff_info->tail_index_offset+length-circle_buff_info->buff_length)
                    ,length-(circle_buff_info->buff_length-circle_buff_info->tail_index_offset));

            circle_buff_info->tail_index_offset =
                        length-(circle_buff_info->buff_length-circle_buff_info->tail_index_offset);
        }else{
            memcpy(circle_buff_info->buff+circle_buff_info->tail_index_offset,push_ptr,length);
            circle_buff_info->tail_index_offset += circle_buff_info->element_length;
        }

    }else{
            memcpy(circle_buff_info->buff+circle_buff_info->tail_index_offset,push_ptr,length);
            circle_buff_info->tail_index_offset += circle_buff_info->element_length;
    }
    printf("head:%d tail:%d\n",circle_buff_info->head_index_offset, circle_buff_info->tail_index_offset);
    return ret_val;
}

/*push all items per operation*/
int32_t push_circle_buff_bundle(buffer_info_t *circle_buff_info, buffer_info_t *local_buff)
{
    int ir;
    int iw;
    int or;
    int ow;
/*
    ir = circle_buff_info->head_index_offset;
    iw = circle_buff_info->tail_index_offset;
    or = local_buff->head_index_offset;
    ow = local_buff->tail_index_offset;
    */
    //printf("ir,iw,or,ow:%d %d %d %d\r\n",ir,iw,or,ow);
    ir = local_buff->head_index_offset;
    iw = local_buff->tail_index_offset;
    or = circle_buff_info->head_index_offset;
    ow = circle_buff_info->tail_index_offset;
    if(ir<iw)
    {
        if((circle_buff_info->buff_length-ow)>(iw-ir))
        {
            /*
             * in      |    R------W |temp_var_info
             * local   |   W         |
             * res     |   -------W  |
             */
            //printf("buff_len:%d,data_len:%d\r\n",1024*1024,iw-ir);
            memcpy(&(circle_buff_info->buff[ow]), &local_buff->buff[ir], (iw-ir));

        }else{
            /*
             * in      |    R------W |
             * local   |        W    |
             * res     |-W      -----|
             */
            memcpy(&circle_buff_info->buff[ow], &local_buff->buff[ir], (circle_buff_info->buff_length-ow));
            memcpy(&circle_buff_info->buff[0], &local_buff->buff[ir+(circle_buff_info->buff_length-ow)], iw-ir-(circle_buff_info->buff_length-ow));

        }
        local_buff->head_index_offset = local_buff->tail_index_offset;
        circle_buff_info->tail_index_offset = (circle_buff_info->tail_index_offset+(iw-ir))%circle_buff_info->buff_length;
        return (iw-ir);

    }else{

        if((circle_buff_info->buff_length-ow)>=(local_buff->buff_length-ir+iw))
        {
            /*
             * in      |+++W     R---|
             * local   |      W      |
             * res     |      ---+++W|
             */
            memcpy(&circle_buff_info->buff[ow], &local_buff->buff[ir], (local_buff->buff_length-ir));
            memcpy(&circle_buff_info->buff[ow+(local_buff->buff_length-ir)], &local_buff->buff[0], iw);

        }else if((circle_buff_info->buff_length-ow)>=(local_buff->buff_length-ir)){
            /*
             * in      |+++W     R---|
             * local   |        W    |
             * res     |+W      ---++|
             */
            memcpy(&circle_buff_info->buff[ow], &local_buff->buff[ir], (local_buff->buff_length-ir));
            memcpy(&circle_buff_info->buff[ow+(local_buff->buff_length-ir)], &local_buff->buff[0], (circle_buff_info->buff_length-ow)-(local_buff->buff_length-ir));
            memcpy(&circle_buff_info->buff[0], &local_buff->buff[(circle_buff_info->buff_length-ow)-(local_buff->buff_length-ir)], (local_buff->buff_length-ir+iw)-(circle_buff_info->buff_length-ow));

        }else if((circle_buff_info->buff_length-ow)<(local_buff->buff_length-ir)){
            /*
             * in      |+++W     R---|
             * local   |           W |
             * res     |-+++W      --|
             */
            memcpy(&circle_buff_info->buff[ow], &local_buff->buff[ir], (circle_buff_info->buff_length-ow));
            memcpy(&circle_buff_info->buff[0], &local_buff->buff[ir+(circle_buff_info->buff_length-ow)], (local_buff->buff_length-ir)-(circle_buff_info->buff_length-ow));
            memcpy(&circle_buff_info->buff[(local_buff->buff_length-ir)-(circle_buff_info->buff_length-ow)], &local_buff->buff[0], iw);

        }
        local_buff->head_index_offset = local_buff->tail_index_offset;
        circle_buff_info->tail_index_offset = (circle_buff_info->tail_index_offset+(local_buff->buff_length-ir+iw))%circle_buff_info->buff_length;
        return (local_buff->buff_length-ir+iw);
    }
    return -1;
}


int32_t pull_circle_buff_item(buffer_info_t *circle_buff_info, void *pull_ptr)
{
    if(circle_buff_info->head_index_offset != circle_buff_info->tail_index_offset)
    {
        //printf("local_buff.r:%d local_buff.w:%d\r\n",local_buff.r,local_buff.w);
        memcpy(pull_ptr,&circle_buff_info->buff[circle_buff_info->head_index_offset],circle_buff_info->element_length);
        circle_buff_info->head_index_offset = (circle_buff_info->head_index_offset+circle_buff_info->element_length)%circle_buff_info->buff_length;
        return 0;
    }
    return -1;//buffer empty
}

/*pull all data per operation*/
int32_t pull_circle_buff_bundle(buffer_info_t *circle_buff_info, buffer_info_t *local_buff)
{
    int ir;
    int iw;
    int or;
    int ow;

    ir = circle_buff_info->head_index_offset;
    iw = circle_buff_info->tail_index_offset;
    or = local_buff->head_index_offset;
    ow = local_buff->tail_index_offset;
    //printf("ir,iw,or,ow:%d %d %d %d\r\n",ir,iw,or,ow);

    if(ir<iw)
    {
        if((local_buff->buff_length-ow)>(iw-ir))
        {
            /*
             * in      |    R------W |temp_var_info
             * local   |   W         |
             * res     |   -------W  |
             */
            //printf("buff_len:%d,data_len:%d\r\n",1024*1024,iw-ir);
            memcpy(&(local_buff->buff[ow]), &circle_buff_info->buff[ir], (iw-ir));

        }else{
            /*
             * in      |    R------W |
             * local   |        W    |
             * res     |-W      -----|
             */
            memcpy(&local_buff->buff[ow], &circle_buff_info->buff[ir], (local_buff->buff_length-ow));
            memcpy(&local_buff->buff[0], &circle_buff_info->buff[ir+(local_buff->buff_length-ow)], iw-ir-(local_buff->buff_length-ow));

        }
        circle_buff_info->head_index_offset = circle_buff_info->tail_index_offset;
        local_buff->tail_index_offset = (local_buff->tail_index_offset+(iw-ir))%local_buff->buff_length;
        return (iw-ir);

    }else{

        if((local_buff->buff_length-ow)>=(circle_buff_info->buff_length-ir+iw))
        {
            /*
             * in      |+++W     R---|
             * local   |      W      |
             * res     |      ---+++W|
             */
            memcpy(&local_buff->buff[ow], &circle_buff_info->buff[ir], (circle_buff_info->buff_length-ir));
            memcpy(&local_buff->buff[ow+(circle_buff_info->buff_length-ir)], &circle_buff_info->buff[0], iw);

        }else if((local_buff->buff_length-ow)>=(circle_buff_info->buff_length-ir)){
            /*
             * in      |+++W     R---|
             * local   |        W    |
             * res     |+W      ---++|
             */
            memcpy(&local_buff->buff[ow], &circle_buff_info->buff[ir], (circle_buff_info->buff_length-ir));
            memcpy(&local_buff->buff[ow+(circle_buff_info->buff_length-ir)], &circle_buff_info->buff[0], (local_buff->buff_length-ow)-(circle_buff_info->buff_length-ir));
            memcpy(&local_buff->buff[0], &circle_buff_info->buff[(local_buff->buff_length-ow)-(circle_buff_info->buff_length-ir)], (circle_buff_info->buff_length-ir+iw)-(local_buff->buff_length-ow));

        }else if((local_buff->buff_length-ow)<(circle_buff_info->buff_length-ir)){
            /*
             * in      |+++W     R---|
             * local   |           W |
             * res     |-+++W      --|
             */
            memcpy(&local_buff->buff[ow], &circle_buff_info->buff[ir], (local_buff->buff_length-ow));
            memcpy(&local_buff->buff[0], &circle_buff_info->buff[ir+(local_buff->buff_length-ow)], (circle_buff_info->buff_length-ir)-(local_buff->buff_length-ow));
            memcpy(&local_buff->buff[(circle_buff_info->buff_length-ir)-(local_buff->buff_length-ow)], &circle_buff_info->buff[0], iw);

        }
        circle_buff_info->head_index_offset = circle_buff_info->tail_index_offset;
        local_buff->tail_index_offset = (local_buff->tail_index_offset+(circle_buff_info->buff_length-ir+iw))%local_buff->buff_length;
        return (circle_buff_info->buff_length-ir+iw);
    }
    return -1;
}

int32_t is_buff_full(buffer_info_t *circle_buff_info)
{
    if(((circle_buff_info->tail_index_offset+circle_buff_info->element_length)
            %circle_buff_info->buff_length) == circle_buff_info->head_index_offset)
    {
        return 1;
    }
    return 0;
}

int32_t is_buff_empty(buffer_info_t *circle_buff_info)
{
    if(circle_buff_info->tail_index_offset
             == circle_buff_info->head_index_offset)
    {
        return 1;
    }
    return 0;
}