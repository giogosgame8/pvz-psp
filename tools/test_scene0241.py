import unittest,struct,tempfile
from pathlib import Path
from PIL import Image
from prepare_scene0241 import source_canvas,support_offset,swimming_specs,write_support_offsets,PLANT_BANKS
from affine_render import composite_affine,AffineLayerCache
from prepare_assets import hidden_track

class ScenePlacementTests(unittest.TestCase):
    def test_negative_boss_coordinates_survive_rasterization(self):
        size,(x,y)=source_canvas('Zombie_boss');canvas=Image.new('RGBA',size)
        layer=Image.new('RGBA',(10,10),'red');matrix=(1,0,0,1,-382+x,-593+y)
        composite_affine(canvas,layer,matrix,(1,0,382-x,0,1,593-y),1,AffineLayerCache())
        self.assertIsNotNone(canvas.getbbox())
        self.assertEqual(canvas.getpixel((x-380,y-590)),(255,0,0,255))

    def test_attachment_preserves_source_cell_despite_asymmetric_crops(self):
        geometry={'pot':((101,100,172,170),(47,46)), 'cabbage':((69,83,166,171),(64,58))}
        for name,height in (('pot',26),('cabbage',-5)):
            box,size=geometry[name];dx,dy=support_offset(name,geometry)
            # Recover the source position of the original cell's origin.
            left=dx-size[0]*.68/2;top=dy-size[1]*.68
            self.assertAlmostEqual(left-(box[0]-96)*size[0]/(box[2]-box[0])*.68,-40*size[0]/(box[2]-box[0])*.68)
            self.assertAlmostEqual(top-(box[1]-96)*size[1]/(box[3]-box[1])*.68,(height-80)*size[1]/(box[3]-box[1])*.68)

    def test_all_potted_entries_are_written_in_seed_order(self):
        geometry={name:((96,96,176,176),(53,53)) for name in PLANT_BANKS}
        with tempfile.TemporaryDirectory() as tmp:
            report=write_support_offsets(Path(tmp),geometry);raw=(Path(tmp)/report['file']).read_bytes()
            self.assertEqual(struct.unpack_from('<4sI',raw),(b'PVPO',50))
            self.assertEqual(len(raw),408)
            self.assertGreater(struct.unpack_from('<2f',raw,8+33*8)[1],0)

    def test_swimming_replaces_walk_for_plain_and_armoured_damage_banks(self):
        samples=[('duck_walk','Zombie','anim_walk',.48,'duck'),('duck_bucket_walk_damage1','Zombie','anim_walk',.48,dict(gear='duck_bucket',armor=1)),('zombie_walk','Zombie','anim_walk',.48,'')]
        result=swimming_specs(samples)
        self.assertEqual([s[2] for s in result],['anim_swim','anim_swim','anim_walk'])
        for gear in ('duck','duck_cone','duck_bucket'):
            self.assertTrue(hidden_track('Zombie_outerleg_foot',gear))
            self.assertFalse(hidden_track('Zombie_whitewater',gear))
            self.assertFalse(hidden_track('Zombie_duckytube',gear))
        self.assertFalse(hidden_track('Zombie_outerleg_foot',''))

    def test_special_plant_height_offsets_are_preserved_on_pots(self):
        for name,height in (('star',5),('pumpkin',10),('coffee',-25),('puff',0),('scaredy',-14),('spikerock',6)):
            geometry={name:((96,96,176,176),(53,53))}
            self.assertAlmostEqual(support_offset(name,geometry)[1],height*53/80*.68)

if __name__=='__main__':unittest.main()
