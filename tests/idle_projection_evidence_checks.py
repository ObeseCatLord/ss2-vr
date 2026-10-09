import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_idle_weapon import assess
from idle_weapon_evidence_checks import geometry_fixture,SOURCE
from idle_stream_evidence_checks import fixture as rejected_fixture

BASE='request=100 eye=1 hand=0'
M=[0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000,0]
P=[0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000]

def producer(flags=0):
    lines=['Lab idle projectionSummary '+BASE+' configured=1 count=1 invalidations=0 blocked=0 pending=0',
           'Lab idle projectionPair '+BASE+f' sequence=1 source=1 complete=1 controlBefore=127 controlAfter=127 flagsBefore={flags} flagsAfter=6 modelBefore=1000 modelAfter=1000 drawBefore=2000 drawAfter=2000 cleanupCertified=0 outerCurrent=0']
    for phase in (0,1):
        for name,values in (('model',M),('view',M),('projection',P),('cachedVP',P),('cachedMVP',P)):
            lines.append('Lab idle projectionData '+BASE+f' sequence=1 phase={phase} kind={name} values='+','.join(f'{v:08x}' for v in values))
    return lines

def fixture(retained=False,flags=0):
    text,row,header,data=geometry_fixture()
    lines=text.splitlines();lines=[v.replace('constants=1','constants=5') for v in lines]
    lines += [row('constant',i,P[(i-1)*4:i*4]) for i in range(1,5)]
    factor='Lab idle geometryFactors '+BASE+' index=0 palette=0 bookends=3 postOriginal=1 cleanupCertified=0 outerCurrent=0'
    assoc='Lab idle geometryProjection '+BASE+' index=0 sequence=1 modelAddress=1000 drawAddress=2000 bookends=3 postOriginal=1 cleanupCertified=0 outerCurrent=0'
    extras=[factor,*[row(name,0,v) for name,v in (('factorModel',M),('factorLocal',M),('factorView',M),('factorProjection',P))],assoc]
    if retained:
        head=rejected_fixture().replace('eye=0','eye=1').replace('hand=1','hand=0').replace('draws=0','draws=1').replace('resource=1','resource=5')
        lines=[head,'Lab idle retainedCopies '+BASE+' count=1 postOriginal=1 cleanupCertified=0 outerCurrent=0',
               *[v.replace('Lab idle geometry','Lab idle retainedGeometry') for v in lines if v.startswith('Lab idle geometry')]]
        extras=[v.replace('Lab idle geometry','Lab idle retainedGeometry') for v in extras]
        return '\n'.join([*lines,*producer(flags),*extras])
    return '\n'.join([*lines,*producer(flags),*extras])

class Checks(unittest.TestCase):
    def test_poly_bump_source_is_diagnostic_only(self):
        from idle_native_reference import native_reference
        text=fixture().replace('source=1 complete=1','source=2 complete=1')
        record=assess(text,SOURCE)['copied_event_pose_observations'][0]
        pair=record['projection_probe']['pairs'][1]
        self.assertEqual(pair['source'],2)
        self.assertTrue(pair['diagnostic_only']);self.assertFalse(record['projection_probe']['alignment_accepted'])
        self.assertIsNone(native_reference(record,record['geometry'][0]))
    def test_schema4_associations_cover_last_two_indices_without_weakening_keys(self):
        lines=fixture().splitlines()
        geometry=[line for line in lines if line.startswith('Lab idle geometry')]
        ten=[line.replace('schema=3 ','schema=4 copyLayout=1 ').replace('draws=1','draws=10')
             for line in lines if not line.startswith('Lab idle geometry')]
        for i in range(10):ten.extend(line.replace('index=0 ','index='+str(i)+' ') for line in geometry)
        text='\n'.join(ten);e=assess(text,SOURCE)
        g=e['copied_event_pose_observations'][0]['geometry']
        self.assertEqual(set(g),set(range(10)))
        for i in (8,9):
            self.assertEqual(g[i]['projection_association']['sequence'],1)
            self.assertFalse(g[i]['projection_association']['alignment_accepted'])
            target='index='+str(i)+' sequence=1 modelAddress=1000 drawAddress=2000'
            for bad in (target.replace('drawAddress=2000','drawAddress=2001'),
                        target.replace('sequence=1','sequence=2')):
                with self.assertRaises(ValueError):assess(text.replace(target,bad),SOURCE)
        with self.assertRaises(ValueError):assess(text.replace('index=9 ','index=10 '),SOURCE)

    def test_cold_and_cached_modes_preserve_false_reference_claims(self):
        for retained in (False,True):
            for flags in (0,2,4,6):
                result=assess(fixture(retained,flags),SOURCE)
                r=result['rejected_or_missing_observations' if retained else 'copied_event_pose_observations'][0]
                p=r['projection_probe']['pairs'][1]
                self.assertEqual(p['precision_bits'],0);self.assertEqual(p['rounding_bits'],0)
                self.assertEqual(p['vp_computed'],not bool(flags&2));self.assertEqual(p['mvp_computed'],not bool(flags&4))
                self.assertFalse(p['earlier_vp_producer_verified']);self.assertFalse(p['earlier_mvp_producer_verified'])
                g=(r['retained_copies']['geometry'] if retained else r['geometry'])[0]
                self.assertFalse(g['projection_association']['alignment_accepted'])
                self.assertFalse(g['projection_association']['reference_replaced'])
                self.assertFalse(result['alignment_accepted']);self.assertFalse(result['positive_grasp_verified'])
    def test_all_operands_are_complete_and_raw_bits(self):
        text=fixture();rows=text.splitlines()
        for i,line in enumerate(rows):
            if 'projectionData ' in line:
                with self.subTest(line=line),self.assertRaises(ValueError):assess('\n'.join(rows[:i]+rows[i+1:]),SOURCE)
        for phase in (0,1):
            for kind in ('model','view','projection'):
                target='phase='+str(phase)+' kind='+kind+' values=3f800000'
                with self.subTest(phase=phase,kind=kind),self.assertRaises(ValueError):
                    assess(text.replace(target,target[:-8]+'3f800001'),SOURCE)
    def test_missing_duplicate_unknown_foreign_and_unretired_reject(self):
        text=fixture();rows=text.splitlines()
        pair=next(v for v in rows if 'projectionPair ' in v)
        summary=next(v for v in rows if 'projectionSummary ' in v)
        for bad in (text+'\n'+pair,text+'\n'+summary,text.replace('configured=1 count=1','configured=0 count=1'),
                    text.replace('pending=0','pending=1'),text.replace('blocked=0','blocked=1'),
                    text.replace('sequence=1 source=1','sequence=2 source=1'),text.replace('source=1 complete=1','source=3 complete=1'),
                    text.replace('controlAfter=127','controlAfter=383'),text.replace('flagsAfter=6','flagsAfter=2'),
                    text.replace('modelAfter=1000','modelAfter=1001'),text.replace('drawAfter=2000','drawAfter=2001'),
                    text.replace('kind=cachedVP','kind=unknown'),text.replace('projectionPair '+BASE,'projectionPair request=100 eye=0 hand=0'),
                    text.replace('projectionData '+BASE+' sequence=1 phase=1','projectionData '+BASE+' sequence=1 phase=2'),
                    text.replace('outerCurrent=0','outerCurrent=1')):
            with self.subTest(bad=bad[-180:]),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_association_cannot_borrow_other_operands_or_constants(self):
        text=fixture()
        for bad in (text.replace('modelAddress=1000','modelAddress=1001'),text.replace('drawAddress=2000','drawAddress=2001'),
                    text.replace('index=0 sequence=1 modelAddress','index=0 sequence=2 modelAddress'),
                    text.replace('kind=factorView chunk=0 values=3f800000','kind=factorView chunk=0 values=3f800001'),
                    text.replace('kind=constant chunk=4 values=00000000,00000000,00000000,3f800000',
                                 'kind=constant chunk=4 values=00000000,00000000,00000000,3f800001')):
            with self.assertRaises(ValueError):assess(bad,SOURCE)
        rows=text.splitlines();assoc=next(v for v in rows if 'geometryProjection ' in v)
        with self.assertRaises(ValueError):assess(text+'\n'+assoc,SOURCE)
        with self.assertRaises(ValueError):assess('\n'.join(v for v in rows if 'geometryFactors ' not in v),SOURCE)
    def test_optional_evidence_can_be_absent_and_does_not_change_geometry_data(self):
        text=fixture();base='\n'.join(v for v in text.splitlines() if not ('projection' in v.split()[2] or 'geometryProjection ' in v))
        a=assess(text,SOURCE)['copied_event_pose_observations'][0];b=assess(base,SOURCE)['copied_event_pose_observations'][0]
        self.assertEqual(a['geometry'][0]['data'],b['geometry'][0]['data'])
        self.assertEqual(a['geometry_observed'],b['geometry_observed'])
    def test_all_precision_and_rounding_encodings_remain_raw(self):
        for pc in range(4):
            for rc in range(4):
                word=127|(pc<<8)|(rc<<10)
                text=fixture().replace('controlBefore=127 controlAfter=127',f'controlBefore={word} controlAfter={word}')
                p=assess(text,SOURCE)['copied_event_pose_observations'][0]['projection_probe']['pairs'][1]
                self.assertEqual((p['precision_bits'],p['rounding_bits']),(pc,rc))
                self.assertFalse(p['reference_replaced'])
    def test_reuse_must_preserve_cache_words_and_unrelated_flags(self):
        for flags,name in ((2,'cachedVP'),(4,'cachedMVP'),(6,'cachedVP'),(6,'cachedMVP')):
            text=fixture(flags=flags)
            target='phase=0 kind='+name+' values=3f800000'
            with self.assertRaises(ValueError):assess(text.replace(target,target[:-8]+'3f800001'),SOURCE)
        for flags in (0,2,4,6):
            with self.assertRaises(ValueError):assess(fixture(flags=flags).replace('flagsAfter=6','flagsAfter=14'),SOURCE)
        text=fixture(flags=0)
        for name in ('cachedVP','cachedMVP'):
            target='phase=0 kind='+name+' values=3f800000'
            text=text.replace(target,target[:-8]+'7fc00000')
        self.assertFalse(assess(text,SOURCE)['alignment_accepted']) # Cold previous bytes are not inputs.

if __name__=='__main__':unittest.main()
