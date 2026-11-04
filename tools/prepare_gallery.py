"""Small gallery portraits displayed while full animation banks load."""
import struct
from PIL import Image
from prepare_features09 import first_prepared_frame

PLANTS='pea_idle sunflower wallnut cherry mine snow_idle chomper repeater_idle puff sunshroom fume gravebuster hypno scaredy iceshroom doom lily squash threepeater kelp jalapeno spikeweed torchwood tallnut sea plantern cactus blover split star pumpkin magnet cabbage pot kernel coffee garlic umbrella marigold melon gatling twin gloom cattail winter goldmagnet spikerock cob imitater'.split()
ZOMBIES='zombie_walk cone_walk bucket_walk pole_run flag_walk paper_walk door_walk football_walk dancer_walk backup_walk duck_walk snorkel_walk zamboni_walk bobsled_walk dolphin_walk jack_walk balloon_walk digger_walk pogo_walk yeti_walk bungee_walk ladder_walk catapult_walk garg_walk imp_walk boss_walk'.split()+['bot_walk']*6
HEADS='pea_head wallnut jalapeno gatling squash tallnut'.split()


def portrait(folder,name,head=None):
    def load(name):
        path=folder/(name+'.pva')
        ax,ay=struct.unpack_from('<2f',path.read_bytes(),32)
        return first_prepared_frame(path),ax,ay
    image,ax,ay=load(name)
    left,top,right,bottom=-ax,-ay,image.width-ax,image.height-ay
    layers=[(image,left,top,1)]
    if head is not None:
        h,hx,hy=load(HEADS[head]);hs=.84 if head==0 else .56;offset=9.7 if head==0 else 23.7
        h=h.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
        x,y=-3.2-(h.width-hx)*hs,-offset-hy*hs
        left=min(left,x);top=min(top,y);right=max(right,x+h.width*hs);bottom=max(bottom,y+h.height*hs)
        layers.append((h,x,y,hs))
    scale=min(.93,87/max(1,right-left),54/max(1,bottom-top))
    out=Image.new('RGBA',(87,54))
    for im,x,y,factor in layers:
        im=im.resize((max(1,round(im.width*scale*factor)),max(1,round(im.height*scale*factor))),Image.Resampling.LANCZOS)
        out.alpha_composite(im,(round(43.5+(x-(left+right)/2)*scale),round(27+(y-(top+bottom)/2)*scale)))
    return out


def prepare_gallery(destination,previews,write_pva):
    result=[]
    for tab,names in [('plants',PLANTS),('zombies',ZOMBIES)]:
        frames=[portrait(destination,name,i-26 if tab=='zombies' and i>=26 else None) for i,name in enumerate(names)]
        name='gallery_'+tab
        result.append(write_pva(destination/(name+'.pva'),frames,1,(0,0)))
        if previews:
            sheet=Image.new('RGBA',(87*8,54*((len(frames)+7)//8)),(45,48,40,255))
            for i,frame in enumerate(frames):sheet.alpha_composite(frame,((i%8)*87,(i//8)*54))
            sheet.save(previews/(name+'.png'))
    return result
