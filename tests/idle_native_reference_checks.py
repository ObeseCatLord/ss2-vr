import copy
from fractions import Fraction
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from idle_native_reference import first_material_matrices, native_reference, round24, UnsupportedNativeArithmetic
from idle_projection_evidence_checks import fixture, M, P, SOURCE
from assess_idle_weapon import assess

def words(values):return list(struct.unpack('<'+str(len(values))+'I',struct.pack('<'+str(len(values))+'f',*values)))

class Checks(unittest.TestCase):
    def test_identity_and_independent_model_translation(self):
        self.assertEqual(first_material_matrices(M,M,P),(P,P))
        model=M.copy();model[3]=0x41200000;model[7]=0xc1200000
        q,r=first_material_matrices(model,M,P)
        self.assertEqual(q,P)
        self.assertEqual(r[:12],model);self.assertEqual(r[12:],P[12:])

    def test_native_add_order_and_precision_cancellation(self):
        view=words([2**24,2**24,0,0,1,1,0,0,-2**24,-2**24,1,0])
        projection=words([1,1,1,0,0,1,0,0,1,1,1,0,0,0,0,1])
        q,_=first_material_matrices(M,view,projection)
        self.assertEqual(q[0],0) # (1+large)+(-large), PC24 stores the tie.
        self.assertEqual(q[9],0x3f800000) # (1+(-large))+large.
        model=words([2**24,0,0,0,1,1,0,0,-2**24,0,1,0])
        _,r=first_material_matrices(model,M,projection)
        self.assertEqual(r[0],0x3f800000) # (-large+large)+1.
        self.assertEqual(round24(Fraction(1)+Fraction(1,2**24)),Fraction(1))
        self.assertEqual(round24(Fraction(1)+Fraction(3,2**24)),Fraction(1)+Fraction(1,2**22))

    def test_float_stores_and_invalid_inputs(self):
        projection=P.copy();projection[0]=1
        view=M.copy();view[0]=0x3f000000
        q,r=first_material_matrices(M,view,projection)
        self.assertEqual((q[0],r[0]),(0,0)) # Binary32 store of half smallest subnormal.
        for bad in (M[:-1],M+[0],[True,*M[1:]],[0x7f800000,*M[1:]],[0x7fc00000,*M[1:]]):
            with self.assertRaises(ValueError):first_material_matrices(bad,M,P)
        huge=words([3e38,0,0,0,0,1,0,0,0,0,1,0])
        projection=P.copy();projection[0]=0x40000000
        with self.assertRaises(ValueError):first_material_matrices(M,huge,projection)

    def test_only_observed_cold_associations_are_supported(self):
        for retained in (False,True):
            evidence=assess(fixture(retained),SOURCE)
            record=evidence['rejected_or_missing_observations' if retained else 'copied_event_pose_observations'][0]
            g=(record['retained_copies']['geometry'] if retained else record['geometry'])[0]
            reference=native_reference(record,g)
            self.assertEqual(reference['matrix'],P);self.assertEqual(reference['local'],M)
            self.assertFalse(reference['cache_outputs_used_as_inputs']);self.assertFalse(reference['alignment_accepted'])
            for flags in (2,4,6):
                other=assess(fixture(retained,flags),SOURCE)
                r=other['rejected_or_missing_observations' if retained else 'copied_event_pose_observations'][0]
                geometry=(r['retained_copies']['geometry'] if retained else r['geometry'])[0]
                self.assertIsNone(native_reference(r,geometry))
            no_association=copy.deepcopy(g);no_association.pop('projection_association')
            self.assertIsNone(native_reference(record,no_association))
            for control in (383,639,895,1151,2175,3199):
                r=copy.deepcopy(record);r['projection_probe']['pairs'][1]['controlBefore']=control
                r['projection_probe']['pairs'][1]['controlAfter']=control
                self.assertIsNone(native_reference(r,g))
            for flags in (1,8,9):
                r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
                p['flagsBefore']=flags;p['flagsAfter']=flags|6
                self.assertIsNone(native_reference(r,g))

    def test_outputs_cannot_seed_the_independent_reference(self):
        record=assess(fixture(),SOURCE)['copied_event_pose_observations'][0]
        g=record['geometry'][0]
        r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
        p['data']['1:cachedVP'][0]+=1
        with self.assertRaises(UnsupportedNativeArithmetic):native_reference(r,r['geometry'][0])
        r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
        p['data']['0:projection'][0]+=1;p['data']['1:projection'][0]+=1
        r['geometry'][0]['data']['factorProjection:0'][0]+=1
        with self.assertRaises(UnsupportedNativeArithmetic):native_reference(r,r['geometry'][0])
        r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
        p['data']['1:cachedVP'][1]=0x80000000 # Signed-zero disagreement rejects.
        with self.assertRaises(UnsupportedNativeArithmetic):native_reference(r,r['geometry'][0])
        r=copy.deepcopy(record);r['geometry'][0]['data']['factorLocal:0'][0]=0x7fc00000
        with self.assertRaises(ValueError):native_reference(r,r['geometry'][0])
        self.assertEqual(native_reference(record,g)['matrix'],P)

if __name__=='__main__':unittest.main()
