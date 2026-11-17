"""Additional authored transitions, baked from the player's disc resources."""
import math

POLISH_SPECS=[
 ('boss_bungee_leave','Zombie_boss','anim_bungee_1_leave',.25,dict(samples=2)),
 ('dancer_moonwalk','Zombie_disco','anim_moonwalk',.48,dict(anchor_parent='dancer_walk',samples=2,sample_loop=True)),
 ('dancer_point','Zombie_disco','anim_point',.48,dict(anchor_parent='dancer_walk',samples=2)),
 ('dancer_armraise','Zombie_disco','anim_armraise',.48,dict(anchor_parent='dancer_walk',samples=2,sample_loop=True)),
 ('backup_armraise','Zombie_backup','anim_armraise',.48,dict(anchor_parent='backup_walk',samples=2,sample_loop=True)),
 ('classic_dancer_armraise','Zombie_dancer','anim_armraise',.48,dict(anchor_parent='classic_dancer_walk',samples=2,sample_loop=True)),
]
for gear in ('cone','bucket'):
 for pose in ('walk','eat'):
  name='duck_'+gear+'_'+pose
  POLISH_SPECS.append((name,'Zombie','anim_'+pose,.48,dict(anchor_parent='duck_'+pose,gear='duck_'+gear)))
  for stage in (1,2):
   POLISH_SPECS.append((name+'_damage'+str(stage),'Zombie','anim_'+pose,.48,dict(base=name,gear='duck_'+gear,armor=stage)))

def sample_transform(frames,index,start,stop,loop=False):
 """Interpolate transforms only; image/visibility keys remain discrete."""
 lo=math.floor(index)
 if lo<start or lo>=stop:raise ValueError('Sample outside its clip')
 current=frames[lo];fraction=index-lo
 if not fraction:return current
 hi=lo+1
 if hi>=stop:hi=start if loop else stop-1
 following=frames[hi]
 result=current.copy()
 for key in ('x','y','kx','ky','sx','sy','a'):
  result[key]=current[key]+(following[key]-current[key])*fraction
 return result
