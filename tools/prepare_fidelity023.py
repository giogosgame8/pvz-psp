"""Owned-disc transition clips; source-canvas anchors keep poses attached."""
FIDELITY_SPECS=[
 ('chomper_swallow','Chomper','anim_swallow',.56,dict(anchor_parent='chomper')),
 ('cabbage_shoot','Cabbagepult','anim_shooting',.66,dict(anchor_parent='cabbage')),
 ('melon_shoot','Melonpult','anim_shooting',.66,dict(anchor_parent='melon')),
 ('kernel_shoot','Cornpult','anim_shooting',.66,dict(anchor_parent='kernel',omit_track=('Cornpult_butter',))),
 ('kernel_butter','Cornpult','anim_shooting',.66,dict(anchor_parent='kernel',omit_track=('Cornpult_kernal',))),
 ('sunshroom_grow','SunShroom','anim_grow',.66,dict(anchor_parent='sunshroom')),
 ('kelp_grab','TangleKelp','anim_grab',.66,dict(anchor_parent='kelp')),
 ('magnet_shoot','MagnetShroom','anim_shooting',.66,dict(anchor_parent='magnet')),
 ('magnet_recharge','MagnetShroom','anim_nonactive_idle',.66,dict(anchor_parent='magnet')),
 ('ladder_place','Zombie_ladder','anim_placeladder',.48,''),
 ('ladder_bare_walk','Zombie_ladder','anim_walk',.48,''),
 ('ladder_bare_eat','Zombie_ladder','anim_eat',.48,''),
 ('balloon_pop','Zombie_balloon','anim_pop',.48,''),
 ('balloon_ground','Zombie_balloon','anim_walk',.48,''),
 ('digger_drill','Zombie_digger','anim_drill',.48,''),
 ('digger_land','Zombie_digger','anim_landing',.48,''),
 ('digger_ground','Zombie_digger','anim_walk',.48,''),
 ('pogo_ground','Zombie_pogo','anim_walk',.48,''),
 ('snorkel_rise','Zombie_snorkle','anim_uptoeat',.48,''),
 ('bungee_hold','Zombie_bungi','anim_idle',.48,''),
]
for pose in ('walk','eat'):
 FIDELITY_SPECS.append(('jack_bare_'+pose,'Zombie_jackbox','anim_'+pose,.48,dict(base='jack_'+pose,omit_track=('Zombie_jackbox_handle','Zombie_jackbox_box','Zombie_jackbox_box2','Zombie_jackbox_clownneck3','Zombie_jackbox_clownneck2','Zombie_jackbox_clownneck1','Zombie_jackbox_clownhead'))))
 FIDELITY_SPECS.append(('football_bare_'+pose,'Zombie_football','anim_'+pose,.48,dict(base='football_'+pose,omit_track=('zombie_football_helmet',))))

# Disarmed bodies keep their own equipment omissions at both damage stages.
for name,source,marker,scale,settings in list(FIDELITY_SPECS):
 if name in ('ladder_bare_walk','ladder_bare_eat','pogo_ground','jack_bare_walk','jack_bare_eat','football_bare_walk','football_bare_eat','balloon_ground','digger_ground'):
  for stage in (1,2):
   gear=dict(settings) if isinstance(settings,dict) else {}
   gear.update(base=name,body=stage)
   FIDELITY_SPECS.append((name+'_damage'+str(stage),source,marker,scale,gear))

# Ordinary advanced bodies share the source's head/outer-arm loss tracks.
for prefix,source in [('duck','Zombie'),('snorkel','Zombie_snorkle'),('dolphin','Zombie_dolphinrider'),('jack','Zombie_jackbox'),('balloon','Zombie_balloon'),('digger','Zombie_digger'),('pogo','Zombie_pogo'),('ladder','Zombie_ladder'),('imp','Zombie_imp'),('yeti','Zombie_yeti')]:
 for pose in ('walk','eat'):
  marker='swim' if prefix=='snorkel' and pose=='walk' else 'ride' if prefix=='dolphin' and pose=='walk' else 'idle' if prefix=='balloon' and pose=='walk' else 'dig' if prefix=='digger' and pose=='walk' else 'pogo' if prefix=='pogo' and pose=='walk' else 'ladderwalk' if prefix=='ladder' and pose=='walk' else 'laddereat' if prefix=='ladder' and pose=='eat' else pose
  for stage in (1,2):FIDELITY_SPECS.append((prefix+'_'+pose+'_damage'+str(stage),source,'anim_'+marker,.48,dict(base=prefix+'_'+pose,gear='duck' if prefix=='duck' else '',body=stage)))
