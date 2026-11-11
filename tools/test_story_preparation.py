import unittest
from PIL import Image
from prepare_story023 import DaveProp

def frame(**values):
 f=dict(x=0,y=0,kx=0,ky=0,sx=1,sy=1,a=1,f=0,i='owned')
 f.update(values);return f
def multiply(m,n):
 a,b,c,d,x,y=m;e,f,g,h,u,v=n
 return a*e+b*g,a*f+b*h,c*e+d*g,c*f+d*h,a*u+b*v+x,c*u+d*v+y
def invert(m):
 a,b,c,d,x,y=m;det=a*d-b*c
 return d/det,-b/det,-c/det,a/det,(b*y-d*x)/det,(c*x-a*y)/det

class StoryPreparationTests(unittest.TestCase):
 def prop(self,kind,children):
  hand=[frame(x=10,y=20) for _ in range(266)];hand[250]=frame(x=15,y=27)
  return DaveProp(kind,{'reanim/wallnut.reanim':'source','reanim/hammer.reanim':'source','reanim/treefood.reanim':'source'},[('Dave_handinghand',hand)],lambda _: (12,children),lambda *_:(0,3),lambda _:Image.new('RGBA',(1,1)),multiply,invert)
 def test_prop_follows_hand_without_adding_reference_translation_twice(self):
  p=self.prop('wallnut',[('body',[frame()]*3)]);draws=[]
  p.draw(None,250,0,lambda canvas,image,m,alpha:draws.append(m))
  self.assertEqual(draws,[(1.2,0,0,1.2,105,400)])
 def test_hammer_holds_last_authored_pose_instead_of_looping(self):
  p=self.prop('hammer',[('body',[frame(x=n) for n in range(3)])]);draws=[]
  p.draw(None,249,20,lambda canvas,image,m,alpha:draws.append(m))
  self.assertEqual(draws[0][4],65)
 def test_treefood_does_not_draw_the_gardening_pour_particle(self):
  p=self.prop('treefood',[('bag',[frame()]),('seed',[frame()])]);draws=[]
  p.draw(None,249,20,lambda *args:draws.append(args))
  self.assertEqual(len(draws),1)

if __name__=='__main__':unittest.main()
