enum { CROW_ROUTE_COUNT=6,CROW_MAX_FRAMES=24 };
typedef struct {unsigned frames,landing,right;int16_t point[CROW_MAX_FRAMES][2];} CrowRoute;
static CrowRoute crow_routes[CROW_ROUTE_COUNT];
static unsigned crow_rate;
static int crow_route=-1;
static float crow_flight=1.1f,crow_landing=1.1f;

static int menu_crow_load_routes(const char *path){
 crow_rate=0;
 FILE *file=fopen(path,"rb");if(!file)return 0;
 uint32_t header[4];unsigned char bytes[CROW_ROUTE_COUNT*(4+CROW_MAX_FRAMES*4)];
 int read_header=fread(header,sizeof(header),1,file)==1;
 size_t length=fread(bytes,1,sizeof(bytes),file);int end=fgetc(file)==EOF&&!ferror(file);fclose(file);
 if(!read_header||!end||header[0]!=0x52435650||header[1]!=1||header[2]!=12||header[3]!=crc32(0,bytes,length))return 0;
 size_t offset=0;
 for(unsigned r=0;r<CROW_ROUTE_COUNT;r++){
  if(length-offset<4)return 0;
  CrowRoute *route=&crow_routes[r];route->frames=bytes[offset];route->landing=bytes[offset+1];route->right=bytes[offset+2];offset+=4;
  if(route->frames<2||route->frames>CROW_MAX_FRAMES||route->landing>=route->frames||route->right>1||length-offset<route->frames*4)return 0;
  memcpy(route->point,bytes+offset,route->frames*4);offset+=route->frames*4;
 }
 if(offset!=length)return 0;
 crow_rate=header[2];return 1;
}
static int crow_route_for(int previous,int page){
 if(!crow_rate)return -1;
 if(previous<0)return page==0?0:page==2?1:-1;
 if(previous==0)return page==1?2:4;
 if(page==0)return previous==1?3:5;
 return -1;
}
static float crow_position(unsigned route_index,int axis,float position){
 const CrowRoute *route=&crow_routes[route_index];
 float key=fminf(route->frames-1,fmaxf(0,position));
 unsigned current=(unsigned)key,next=current+1<route->frames?current+1:current;
 float fraction=key-current;
 return (route->point[current][axis]+fraction*(route->point[next][axis]-route->point[current][axis]))/16.0f;
}
