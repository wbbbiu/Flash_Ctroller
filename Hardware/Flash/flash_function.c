#include "flash_function.h"
#include "use.h"
Flash_Map_t flash_map={.log_cur=0XFFFF,.config_cur=0XFFFF,.config_addrs={0}};
System_Error_t Flash_Config_Write(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len){
    System_Error_t error={.all=0};
    if(index>0x7FF){
        error.bits.module=MOD_FLASH;
        error.bits.errno=SECTOR_INDEX_LARGE;
        return error;
    }
      uint32_t addr=index<<12;
     error=Flash_Write_Sector(spi,addr,data,len);
     return error;
}
System_Error_t Flash_Config_Read(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len){
    System_Error_t error={.all=0};
    if(index>0x7FF){
        error.bits.module=MOD_FLASH;
        error.bits.errno=SECTOR_INDEX_LARGE;
        return error;
    }
      uint32_t addr=index<<12;

     error=Flash_ReadData(spi,addr,data,len);
     return error;
}
//通过二分查找来实现这个查找空闲页面,mode代表查询是config的还是log的,ret存储查询结构
/*
注意这个地方我们为了保证有效,每个页前面都会加上这个header标志的,哪怕你存储的结构体是跨页面的我也是跨页面先写入
header在读取
*/
System_Error_t Flash_Find_Idle(Spi_Device *spi,uint8_t mode,uint32_t *ret){
    int32_t begin, end, middle;
    if(mode==CONFIG_FLASH){
       begin=CONFIG_START_ADDR;
        end=CONFIG_MAX_ADDR;
    }else{
         begin=LOG_START_ADDR;
        end=LOG_MAX_ADDR;
    }
    
    *ret=0xFFFFFFFF;
    System_Error_t error={.all=0};
    while(begin<=end){
        middle=(end-begin)/2+begin;
        uint32_t addr=middle<<8;
        Flash_Header_t flash_header;
        error=Flash_ReadData(spi,addr,(uint8_t*)&flash_header,sizeof(Flash_Header_t));//查询这个页
        if(error.all==0){
            if(flash_header.magic==MAGIC_MASK&&flash_header.id<=STORAGE_ID_MAX){//如果写入了
                begin=middle+1;//那么就往后继续找
            }else{
                if(flash_header.magic==0xFFFF){//遇见没写入的往前找
                    end=middle-1;
                    *ret=middle;
                }else{//明明不是写入但是也不是擦除的,有问题退出
                    break;
                }
            }
        }else{
            return error;
        }
    }
    if(*ret!=0xFFFFFFFF){
        if(CONFIG_FLASH==mode){
            flash_map.config_cur=*ret;
       }else{
           flash_map.log_cur=*ret;
       }
    }else{
        error.bits.module=MOD_FLASH;
        error.bits.errno=FLASH_CONFUSION;
    }
    
    return error;
}
//擦除整个芯片回到最初状态
void Flash_Original(Spi_Device *spi){
    Flash_Erase_Memory(spi,0,ALL);
}
//去读取这个flash中的存储的映射关系记录下来
System_Error_t Flash_Map_Read(Spi_Device *spi,uint32_t *ret){
    System_Error_t error=Flash_Find_Idle(spi,CONFIG_FLASH,ret);//找到这个配置的空闲页开始往回找
    if(error.all!=0){
        return error;
    }
    
    uint32_t addr;
    int32_t payload_per_page=256-sizeof(Flash_Header_t);
    int count=0;
    for(int i=*ret-1;i>=0;i--){//一直到页编号为0
        addr=i<<8;//将页编号换成32为地址
        Flash_Header_t flash_header;
        error=Flash_ReadData(spi,addr,(uint8_t*)&flash_header,sizeof(Flash_Header_t));
        if(flash_header.magic==MAGIC_MASK){//读取出来
            uint16_t id=flash_header.id;
            if(id < STORAGE_ID_MAX &&flash_map.config_addrs[id]==0){//如果这个id还没写入就写入进去查找到结构体+1
                uint32_t total_pages = (flash_header.len + payload_per_page - 1) / payload_per_page;
                uint16_t start_page = i - total_pages + 1; 

                flash_map.config_addrs[id] = start_page;
               count++;
               if(count==STORAGE_ID_MAX){//所有结构体找完退出,此时映射关系表已经有了
                return error;
               }
            }
        }else{
            continue;;
        }
    }
}
//用来进行基础的写操作可以写LOG或者结构体,写LOG时id=STORAGE_ID_MAX,写的过程地址后移动
void Flash_Write_Base(Spi_Device *spi,uint16_t id,uint8_t *data,uint32_t len,uint32_t *write_addr){
    uint32_t payload_per_page = 256 - sizeof(Flash_Header_t);
    uint32_t page_count=(len+payload_per_page-1)/payload_per_page;//知道要想写几个页面
    if(!page_count) page_count=1;
    //每个页面都需要先写入header头,因此记录header头
    Flash_Header_t flash_header={.id=id,.len=len,.magic=MAGIC_MASK};
    uint8_t *flash_ptr=(uint8_t*)(&flash_header);
    const uint16_t flash_header_size=sizeof(Flash_Header_t);
    uint8_t *write_index=data;
    flash_map.config_addrs[id]=(uint16_t)((*write_addr>>8)&0xFFF);//更新这个结构体指向的位置,写入起始点
   
    while(page_count--){//最多写多少页
        Flash_Write_Enable(spi);//写使能
        Spi_Cs_Low(spi);//拉低
        Spi_RW(spi,WRITE_DATA);//发送命令
        Flash_Write_Addr(spi,*write_addr);//给写入地址
        for(int i=0;i<flash_header_size;i++){//先吧头写进去
         Spi_RW(spi,*flash_ptr++);
        }
        flash_ptr=(uint8_t*)&flash_header;
        uint16_t write_len=256-flash_header_size>len?len:256-flash_header_size;
        for(int i=0;i<write_len;i++){//剩余部分写入进入
         Spi_RW(spi,*write_index++);
        }
        len-=write_len;//长度改变
        Spi_Cs_Hight(spi);
        Flash_WaitBusy(spi);
        *write_addr+=0x100;//写地址更新一页
     }

}
//idle 复制到这个空闲区域,当前要复制的结构体的起始页面编号
System_Error_t Flash_Copy_Idle(Spi_Device *spi,uint16_t *idle,uint16_t *start_page){
    if(*idle==IDLE_START_ADDR){//如果是最初空闲说明是一次新的搬运擦除
        Flash_Erase_Memory(spi,(*idle)<<8,BLOCK64);//擦除整个空闲块
    }
    const uint16_t flash_header_size=sizeof(Flash_Header_t);//计算值头
    System_Error_t error={.all=0};
    Flash_Header_t flash_header;
    error=Flash_ReadData(spi,(*start_page)<<8,(uint8_t*)&flash_header,flash_header_size);//读出这个页面的头
    uint32_t payload_per_page = 256 - sizeof(Flash_Header_t);
    uint16_t jump_page=(flash_header.len+ payload_per_page-1)/payload_per_page;//这个结构体要搬运多少页
    uint8_t *temp=(uint8_t *)malloc(256);//一页的大小
    uint32_t write_addr=(uint32_t)(*idle)<<8;//将空闲页编号换为地址
    uint32_t read_addr=(uint32_t)(*start_page)<<8;//读取页面换成地址
    while(jump_page--){//搬运多少页
      Flash_ReadData(spi,read_addr,temp,256);//从起始地读
      read_addr+=0x100;
      Flash_Write_Base(spi,flash_header.id,temp,256,&write_addr);//读出来写回
    }
    flash_map.config_addrs[flash_header.id]=*idle;//记录写入的页面位置
    *start_page=*idle;
    *idle=(write_addr>>8)&0xFF;//更新这个写入后的页面编号
    free(temp);
    temp=NULL;
}
//当你写满写不下去的时候需要重头开始环形缓冲区回到开头,需要将现有的复制一份
void Flash_Begin_Config(Spi_Device *spi){
     uint16_t *begins=(uint16_t*)malloc(sizeof(uint16_t)*STORAGE_ID_MAX);//存放所有结构的的页面编号
     for(int i=0;i<STORAGE_ID_MAX;i++){
        begins[i]=flash_map.config_addrs[i];
     }
     //此处进行从打到小的排序我们从前面开始擦除并搬运的过程可能有的内容一直没更改,在我们擦除写入的页面
     //这种情况我们就要判断如果是当前页面就直接不动否则擦除搬运
     u16_merge_sort(begins,0,STORAGE_ID_MAX-1);
     const uint16_t flash_header_size=sizeof(Flash_Header_t);//计算值头
     System_Error_t error={.all=0};
     uint16_t write_page=0;//写入的页面
     uint16_t erase_flag=0;
     for(int i=0;i<STORAGE_ID_MAX;i++){//依次从起始地址的顺序去处理这些映射关系
        if(!erase_flag){
          for(int j=i;j<STORAGE_ID_MAX;){
            uint16_t copy_index=IDLE_START_ADDR;
            if(begins[i]<write_page+BLOCK_SIZE){//这个结构体存储在当前要擦除的扇区
                Flash_Copy_Idle(spi,&copy_index,&begins[i]);//把这部分内容复制到这个空闲区域
            }else{
                break;
            }
          }
          Flash_Erase_Memory(spi,write_page,BLOCK64);//然后开始擦除扇区
          erase_flag=1;
        }
        //已经保证在当前扇内容复制到空闲,不在当前扇区的保持地址,接下来复制搬运即可
        Flash_Header_t flash_header;
        error=Flash_ReadData(spi,begins[i]<<8,(uint8_t*)&flash_header,flash_header_size);//读出这个页面的头
        uint32_t payload_per_page = 256 - sizeof(Flash_Header_t);
        uint16_t jump_page=(flash_header.len+ payload_per_page-1)/payload_per_page;//根据长度计算跳过几页
        if(((write_page+jump_page)&0xff00)>(write_page&0xFF00)){//跨块了标记需要擦除即可
            erase_flag=0;
            continue;
        }
        //没跨块就开始一页页搬运
            uint8_t *temp=(uint8_t *)malloc(256);//一页的大小
            uint32_t write_addr=(uint32_t)write_page<<8;//给出起始地址
            uint32_t read_addr=(uint32_t)begins[i]<<8;//读的起始地址
            while(jump_page--){//搬运多少页
              Flash_ReadData(spi,read_addr,temp,256);//从起始地读
              read_addr+=0x100;
              Flash_Write_Base(spi,flash_header.id,temp,256,&write_addr);//读出来写回
            }
            flash_map.config_addrs[i]=write_page;//记录写入的页面位置
            write_page=(write_addr>>8)&0xFF;//更新这个写入后的页面编号
            free(temp);
            temp=NULL;
            i++;
     }
     //上面是有效数据全部搬运完了接下来直接擦除使用block和sector
     while(write_page<CONFIG_MAX_ADDR){
        uint32_t erase_addr=write_page<<8;
        if((erase_addr&0xFFF)==0&&(erase_addr+BLOCK_SIZE)<=CONFIG_MAX_ADDR<<8){
            Flash_Erase_Memory(spi,erase_addr,BLOCK64);
            erase_addr+=BLOCK_SIZE;
        }else{
            Flash_Erase_Memory(spi,erase_addr,SECTOR);
            erase_addr+=SECTOR_SIZE;
        }
        write_page=(erase_addr>>8)&0xff;
     }
    }
//log写完回到起点的时候
void Flash_Begin_Log(Spi_Device *spi){
      int copy_start_page=flash_map.log_cur-LOG_MAX_COUNT;
      uint8_t *temp=(uint8_t *)malloc(256);//一页的大小
      uint32_t write_addr=(uint32_t)LOG_START_ADDR<<8;//给出起始地址
      uint32_t read_addr=(uint32_t)copy_start_page<<8;//读的起始地址
      uint32_t erase_flag=0;
      for(int i=0;i<LOG_MAX_COUNT;i++){//最多多少条log复制即可
        if(erase_flag==0){
            Flash_Erase_Memory(spi,write_addr,BLOCK64);
            erase_flag=1;
        }
        Flash_ReadData(spi,read_addr,temp,256);//从起始地读
        read_addr+=0x100;
        Flash_Write_Base(spi,STORAGE_ID_MAX,temp,256,&write_addr);//读出来写回
        if((write_addr&0xFFFF)==0){//到一个新的块
            erase_flag=0;
        }
      }
      flash_map.log_cur=(write_addr>>8)&0xFF;
      uint16_t write_page=flash_map.log_cur;
      while(write_page<CONFIG_MAX_ADDR){
        uint32_t erase_addr=write_page<<8;
        if((erase_addr&0xFFF)==0&&(erase_addr+BLOCK_SIZE)<=CONFIG_MAX_ADDR<<8){
            Flash_Erase_Memory(spi,erase_addr,BLOCK64);
            erase_addr+=BLOCK_SIZE;
        }else{
            Flash_Erase_Memory(spi,erase_addr,SECTOR);
            erase_addr+=SECTOR_SIZE;
        }
        write_page=(erase_addr>>8)&0xff;
     }
}
System_Error_t Flash_Write_Data(Spi_Device *spi,uint16_t id,uint8_t *data,uint32_t len,uint8_t mode){
    uint32_t payload_per_page = 256 - sizeof(Flash_Header_t);
    uint32_t page=(len+payload_per_page-1)/(payload_per_page);

    uint32_t write_addr;
    if(mode==CONFIG_FLASH){
        write_addr=flash_map.config_cur;
        if(write_addr+page>CONFIG_MAX_ADDR){
            Flash_Begin_Config(spi);
        }
    }
    if(mode==LOG_FLASH){
        write_addr=flash_map.log_cur;
        if(write_addr+page>LOG_MAX_ADDR){
            Flash_Begin_Log(spi);
        }
    }
    if(mode==CONFIG_FLASH){
        uint32_t config_write_addr=(uint32_t)flash_map.config_cur<<8;
        Flash_Write_Base(spi,id,data,len,&config_write_addr);
        flash_map.config_cur = (config_write_addr >> 8) & 0xFFFF;
    }
    else{
        uint32_t log_write_addr=(uint32_t)flash_map.log_cur<<8;
        Flash_Write_Base(spi,id,data,len,&log_write_addr);

        flash_map.log_cur=(log_write_addr>>8)&0xFFFF;
        
    }
}
System_Error_t Flash_Read_Config(Spi_Device *spi,uint16_t id,uint8_t *data,uint32_t len,uint8_t mode){
    uint32_t payload_per_page = 256 - sizeof(Flash_Header_t);
    uint32_t page=(len+payload_per_page-1)/(payload_per_page);

    //先确定header大小
    const uint16_t flash_header_size=sizeof(Flash_Header_t);

    uint16_t read_page=flash_map.config_addrs[id];
    while(page--){
        uint32_t read_addr=(uint32_t)read_page<<8;
        Spi_Cs_Low(spi);//拉低
        Spi_RW(spi,READ_DATA);//发送命令
        Flash_Write_Addr(spi,read_addr);//给读地址
        for(int i=0;i<flash_header_size;i++){//先吧头读走
         Spi_RW(spi,READ_MASK);
        }
        uint16_t read_len=256-flash_header_size>len?len:256-flash_header_size;
        for(int i=0;i<read_len;i++,data++){//剩余部读出
         *data=Spi_RW(spi,READ_MASK);
        }
        len-=read_len;//长度改变
        Spi_Cs_Hight(spi);
        read_page+=1;
    }
}
System_Error_t Flash_Read_LOG(Spi_Device *spi,uint8_t *data,uint32_t len,uint8_t mode,uint16_t count){
    //先确定header大小
    const uint16_t flash_header_size=sizeof(Flash_Header_t);

    uint16_t read_page=flash_map.log_cur-count;
   for(int i=read_page;i<flash_map.log_cur;i++){
        uint32_t read_addr=(uint32_t)i<<8;
        Spi_Cs_Low(spi);//拉低
        Spi_RW(spi,READ_DATA);//发送命令
        Flash_Write_Addr(spi,read_addr);//给读地址
        for(int i=0;i<flash_header_size;i++){//先吧头读走
         Spi_RW(spi,READ_MASK);
        }
        uint16_t read_len=256-flash_header_size>len?len:256-flash_header_size;
        for(int i=0;i<read_len;i++,data++){//剩余部读出
         *data=Spi_RW(spi,READ_MASK);
        }
        len-=read_len;//长度改变
        Spi_Cs_Hight(spi);
    }
}