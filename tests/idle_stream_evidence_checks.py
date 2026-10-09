import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_idle_weapon import assess
from idle_stream_evidence import OBSERVED,NO_UV56,OBSERVED_MULTI_UV,PASSIVE_FIVE_ROW78,diagnostic_stream_numbers,declaration_layout

SOURCE='a'*64
BASE='request=100 eye=0 hand=1'

def fixture(declaration=OBSERVED):
    lines=[f'Lab idle draw schema=3 rawGripValid=1 draws=0 source={SOURCE} ipc=10 wire=7 request=100 input=90 owner=1 weapon=2 model=3 generation=4 hand=1 eye=0 stage=3 cfg=20 file=30 resource=1 contributors=1 matrices=4 historicalBytes=0 grasp=0',
           'Lab idle rejection '+BASE+' reason=32 preceding=2 checks=0 state=63 callbacks=63',
           'Lab idle inputFailure '+BASE+' step=20 index=0 hr=0 valid=15 caps=2 declaration=6 rangeChecks=6015']
    lines += ['Lab idle inputDeclaration '+BASE+' index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED)]
    lines += ['Lab idle inputBinding '+BASE+' vertex=70272,0,1,100,0 index=10308,0,1,101,0 draw=4,0,0,218,0,248 software=0']
    streams=[(1,0,12,1),(2,13948,4,1),(1,59328,8,1),(0,0,0,0)]
    for i,v in enumerate(streams):
        lines+=['Lab idle inputStream '+BASE+f' index={i} object={v[0]} offset={v[1]} stride={v[2]} frequency={v[3]}']
    lines+=['Lab idle inputSurface '+BASE+' vertices=218 triangles=248']
    for i,(offset,fmt) in enumerate(((0,133),(0,135),(48384,128),(49256,128))):
        lines+=['Lab idle inputChannel '+BASE+f' index={i} offset={offset} format={fmt} buffer=0']
    lines+=['Lab idle streamProbe '+BASE+' attempts=2 flags=511 words=2 invalidations=0 forwardResult=0']
    for phase in range(2):
        p=BASE+' phase='+str(phase)
        lines+=['Lab idle streamSnapshot '+p+' status=1 step=0 index=0 hr=0',
                'Lab idle streamBinding '+p+' caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=6']
        for i,offset,stride in ((0,0,12),(7,48384,4),(8,49256,4)):
            lines+=['Lab idle streamInput '+p+f' index={i} object=1 offset={offset} stride={stride} frequency=1']
        lines += ['Lab idle streamDeclaration '+p+' index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED)]
        for i in range(2):lines+=['Lab idle streamConstant '+p+f' index={i} values=00000000,00000000,00000000,00000000']
    lines+=['Lab idle streamProgram '+BASE+' chunk=0 values=fffe0101,0000ffff']
    if declaration==NO_UV56:
        lines=[line.replace('declaration=6','declaration=5').replace('rangeChecks=6015','rangeChecks=8191')
               for line in lines if not (('inputDeclaration' in line or 'streamDeclaration' in line) and 'index=5 values=' in line)]
        for n,line in enumerate(lines):
            if 'inputDeclaration' in line or 'streamDeclaration' in line:
                index=int(line.split(' index=')[1].split()[0])
                lines[n]=line.split(' values=')[0]+' values='+','.join(map(str,NO_UV56[index]))
            elif 'inputStream' in line and 'index=1 ' in line:
                lines[n]=line.replace('object=2 offset=13948','object=1 offset=49256')
            elif 'inputStream' in line and 'index=3 ' in line:
                lines[n]=line.replace('object=0 offset=0 stride=0 frequency=0','object=1 offset=48384 stride=4 frequency=1')
            elif 'streamInput' in line:
                lines[n]=line.replace('index=7 object=1 offset=48384','index=5 object=1 offset=49256').replace('index=8 object=1 offset=49256','index=6 object=1 offset=48384')
    elif declaration==PASSIVE_FIVE_ROW78:
        reduced=[]
        for line in lines:
            line=line.replace('declaration=6','declaration=5').replace('rangeChecks=6015','rangeChecks=6143 layout=0')
            if 'inputDeclaration' in line or 'streamDeclaration' in line:
                index=int(line.split(' index=')[1].split()[0])
                if index==2:continue
                new_index=index-1 if index>2 else index
                line=line.split(' index=')[0]+' index='+str(new_index)+' values='+','.join(map(str,PASSIVE_FIVE_ROW78[new_index]))
            reduced.append(line)
        lines=reduced
    elif declaration==OBSERVED_MULTI_UV:
        expanded=[]
        for line in lines:
            line=line.replace('declaration=6','declaration=8')
            if 'inputDeclaration' in line or 'streamDeclaration' in line:
                index=int(line.split(' index=')[1].split()[0])
                prefix=line.split(' index=')[0]
                if index==3:
                    for extra in (3,4):
                        expanded.append(prefix+' index='+str(extra)+' values='+','.join(map(str,OBSERVED_MULTI_UV[extra])))
                new_index=index+2 if index>=3 else index
                line=prefix+' index='+str(new_index)+' values='+','.join(map(str,OBSERVED_MULTI_UV[new_index]))
            expanded.append(line)
        lines=expanded
    return '\n'.join(lines)

class Checks(unittest.TestCase):
    def test_five_row78_passive_with_five_retained_copies_never_promotes(self):
        from idle_weapon_evidence_checks import geometry_fixture
        _,_,header,data=geometry_fixture()
        passive=fixture(PASSIVE_FIVE_ROW78).replace('schema=3 ','schema=3 copyLayout=1 ').replace('draws=0','draws=5')
        payload=[line.replace('eye=1 hand=0','eye=0 hand=1').replace('Lab idle geometry','Lab idle retainedGeometry')
                 for line in [header,*data]]
        companion='Lab idle retainedCopies '+BASE+' count=5 postOriginal=1 cleanupCertified=0 outerCurrent=0'
        copied=[line.replace('index=0 ','index='+str(i)+' ') for i in range(5) for line in payload]
        text='\n'.join([passive,companion,*copied]);result=assess(text,SOURCE)
        self.assertFalse(result['copied_event_pose_observations'])
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(record['stage'],3)
        self.assertEqual(record['rejection']['reason'],32)
        self.assertEqual(set(record['retained_copies']['geometry']),set(range(5)))
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        for field in ('cleanup_certified','outer_current','whole_trace_accepted','positive_grasp_verified','alignment_accepted'):
            self.assertIs(record['retained_copies'][field],False)
        for bad in (passive,text.replace(companion,''),text.replace('count=5 postOriginal','count=4 postOriginal'),
                    text.replace(copied[-1],''),text.replace('cleanupCertified=0','cleanupCertified=1'),
                    text.replace('outerCurrent=0','outerCurrent=1')):
            with self.subTest(case=bad[-80:]),self.assertRaises(ValueError):assess(bad,SOURCE)

    def test_five_row78_is_exact_and_passive_only(self):
        text=fixture(PASSIVE_FIVE_ROW78);result=assess(text,SOURCE)
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(diagnostic_stream_numbers(record['input_failure']),(0,7,8))
        self.assertEqual(set(record['stream_probe']['snapshots'][0]['streams']),{0,7,8})
        self.assertFalse(result['copied_event_pose_observations'])
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        with self.assertRaises(ValueError):declaration_layout(PASSIVE_FIVE_ROW78)
        for index in range(5):
            for field in range(6):
                rows=copy.deepcopy(PASSIVE_FIVE_ROW78);rows[index][field]+=1
                with self.subTest(index=index,field=field),self.assertRaises(ValueError):
                    diagnostic_stream_numbers({'declaration':5,'declaration_rows':dict(enumerate(rows))})
        for count in (4,6,66):
            with self.assertRaises(ValueError):diagnostic_stream_numbers({'declaration':count,'declaration_rows':dict(enumerate(PASSIVE_FIVE_ROW78))})
        for foreign in (2,3,5,6):
            with self.assertRaises(ValueError):assess(text.replace('phase=0 index=7 object','phase=0 index='+str(foreign)+' object'),SOURCE)

    def test_five_row78_keeps_original_selection_and_failure_bounds(self):
        text=fixture(PASSIVE_FIVE_ROW78)
        head=text[:text.index('Lab idle streamSnapshot')].replace('attempts=2 flags=511 words=2','attempts=1 flags=284 words=0')
        partial='Lab idle streamSnapshot '+BASE+' phase=0 status=2 step=9 index=7 hr=-1'
        self.assertFalse(assess(head+partial,SOURCE)['copied_event_pose_observations'])
        for foreign in (2,3,5,6):
            with self.assertRaises(ValueError):assess(head+partial.replace('index=7','index='+str(foreign)),SOURCE)
        changed=[line for line in text.replace('flags=511','flags=447').splitlines() if not ('streamDeclaration '+BASE+' phase=1' in line)]
        changed+=['Lab idle streamDeclaration '+BASE+' phase=1 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(NO_UV56)]
        changed='\n'.join(changed)
        record=assess(changed,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(set(record['stream_probe']['snapshots'][1]['streams']),{0,7,8})
        self.assertFalse(record['stream_probe']['flags']&64)
        with self.assertRaises(ValueError):assess(changed.replace('flags=447','flags=511'),SOURCE)
        with self.assertRaises(ValueError):assess(changed.replace('phase=1 index=7 object','phase=1 index=5 object'),SOURCE)
    def test_multi_uv_passive_receipt_stays_diagnostic_with_exact_copy_family(self):
        text=fixture(OBSERVED_MULTI_UV).replace('rangeChecks=6015','rangeChecks=6143 layout=0');result=assess(text,SOURCE)
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(diagnostic_stream_numbers(record['input_failure']),(0,7,8))
        self.assertEqual(set(record['stream_probe']['snapshots'][0]['streams']),{0,7,8})
        self.assertEqual(record['stream_probe']['flags'],511)
        self.assertFalse(result['copied_event_pose_observations'])
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        self.assertEqual(declaration_layout(OBSERVED_MULTI_UV),(3,True))
        new=text.replace('layout=0','layout=3')
        self.assertEqual(assess(new,SOURCE)['rejected_or_missing_observations'][0]['input_failure']['layout'],3)
        with self.assertRaises(ValueError):assess(new.replace('layout=3','layout=4'),SOURCE)
        for index in range(8):
            for field in range(6):
                rows=copy.deepcopy(OBSERVED_MULTI_UV);rows[index][field]+=1
                with self.subTest(index=index,field=field),self.assertRaises(ValueError):
                    diagnostic_stream_numbers({'declaration':8,'declaration_rows':dict(enumerate(rows))})
        for count in (7,9):
            with self.assertRaises(ValueError):diagnostic_stream_numbers({'declaration':count,'declaration_rows':dict(enumerate(OBSERVED_MULTI_UV))})
        for foreign in (4,5):
            with self.assertRaises(ValueError):assess(text.replace('streamInput '+BASE+' phase=0 index=7','streamInput '+BASE+' phase=0 index='+str(foreign)),SOURCE)
    def test_multi_uv_resampling_cannot_reselect_or_inherit_equality(self):
        text=fixture(OBSERVED_MULTI_UV).replace('flags=511','flags=447')
        rows=[line for line in text.splitlines() if not ('streamDeclaration '+BASE+' phase=1' in line)]
        rows=[line.replace('phase=1 caps=2 declaration=8','phase=1 caps=2 declaration=5') for line in rows]
        rows+=['Lab idle streamDeclaration '+BASE+' phase=1 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(NO_UV56)]
        changed='\n'.join(rows)
        r=assess(changed,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(set(r['stream_probe']['snapshots'][1]['streams']),{0,7,8})
        self.assertFalse(r['stream_probe']['flags']&64)
        with self.assertRaises(ValueError):assess(changed.replace('phase=1 index=7 object','phase=1 index=5 object'),SOURCE)
        with self.assertRaises(ValueError):assess(changed.replace('flags=447','flags=511'),SOURCE)
    def test_exact_no_uv_selection_and_original_binding_agreement(self):
        text=fixture(NO_UV56);result=assess(text,SOURCE)
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(diagnostic_stream_numbers(record['input_failure']),(0,5,6))
        self.assertEqual(set(record['stream_probe']['snapshots'][0]['streams']),{0,5,6})
        self.assertFalse(result['copied_event_pose_observations'])
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        for bad in (text.replace('inputStream '+BASE+' index=1 object=1 offset=49256','inputStream '+BASE+' index=1 object=1 offset=49260'),
                    text.replace('inputStream '+BASE+' index=3 object=1 offset=48384','inputStream '+BASE+' index=3 object=2 offset=48384')):
            with self.assertRaises(ValueError):assess(bad,SOURCE)
        for rows in ([],NO_UV56[:-1],OBSERVED+NO_UV56):
            with self.assertRaises(ValueError):diagnostic_stream_numbers({'declaration':len(rows),'declaration_rows':dict(enumerate(rows))})
        for index in range(5):
            for field in range(6):
                rows=copy.deepcopy(NO_UV56);rows[index][field]+=1
                with self.subTest(index=index,field=field),self.assertRaises(ValueError):
                    diagnostic_stream_numbers({'declaration':5,'declaration_rows':dict(enumerate(rows))})
    def test_no_uv_partial_and_cross_family_indices(self):
        for declaration,valid,foreign in ((NO_UV56,5,7),(OBSERVED,7,5)):
            text=fixture(declaration);head=text[:text.index('Lab idle streamSnapshot')]
            head=head.replace('attempts=2 flags=511 words=2','attempts=1 flags=284 words=0')
            row='Lab idle streamSnapshot '+BASE+f' phase=0 status=2 step=9 index={valid} hr=-1'
            self.assertFalse(assess(head+row,SOURCE)['copied_event_pose_observations'])
            with self.assertRaises(ValueError):assess(head+row.replace('index='+str(valid),'index='+str(foreign)),SOURCE)
        text=fixture(NO_UV56)
        for foreign in (7,8):
            with self.assertRaises(ValueError):assess(text.replace('streamInput '+BASE+' phase=0 index=5','streamInput '+BASE+' phase=0 index='+str(foreign)),SOURCE)
    def test_changed_after_declaration_does_not_reselect_numbers(self):
        text=fixture(NO_UV56).replace('flags=511','flags=447')
        rows=[line for line in text.splitlines() if not ('streamDeclaration '+BASE+' phase=1' in line)]
        rows=[line.replace('phase=1 caps=2 declaration=5','phase=1 caps=2 declaration=6') for line in rows]
        rows+=['Lab idle streamDeclaration '+BASE+' phase=1 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED)]
        changed='\n'.join(rows)
        r=assess(changed,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(set(r['stream_probe']['snapshots'][1]['streams']),{0,5,6})
        self.assertFalse(r['stream_probe']['flags']&64)
        with self.assertRaises(ValueError):assess(changed.replace('phase=1 index=5 object','phase=1 index=7 object'),SOURCE)
        with self.assertRaises(ValueError):assess(changed.replace('flags=447','flags=511'),SOURCE)
    def test_complete_diagnostic_never_promotes_or_infers_roles(self):
        r=assess(fixture(),SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        p=r['rejected_or_missing_observations'][0]['stream_probe']
        self.assertEqual(p['flags'],511)
        self.assertEqual(p['snapshots'][0]['streams'][7]['offset'],48384)
        for k in ('geometry_admitted','roles_inferred','alignment_accepted'):self.assertIs(p[k],False)
    def test_partial_snapshot_cannot_expose_outputs(self):
        text=fixture();start=text.index('Lab idle streamSnapshot')
        head=text[:start].replace('attempts=2 flags=511 words=2','attempts=1 flags=284 words=0')
        partial='Lab idle streamSnapshot '+BASE+' phase=0 status=2 step=9 index=7 hr=-1'
        self.assertEqual(assess(head+partial,SOURCE)['rejected_or_missing_observations'][0]['stream_probe']['snapshots'][0]['status'],2)
        bad=partial+'\nLab idle streamBinding '+BASE+' phase=0 caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=6'
        with self.assertRaises(ValueError):assess(head+bad,SOURCE)
        with self.assertRaises(ValueError):assess(head+partial.replace('hr=-1','hr=0'),SOURCE)
    def test_interrupted_call_has_no_qualified_hresult(self):
        text=fixture();head=text[:text.index('Lab idle streamSnapshot')].replace('attempts=2 flags=511 words=2 invalidations=0','attempts=1 flags=0 words=0 invalidations=2')
        row='Lab idle streamSnapshot '+BASE+' phase=0 status=3 step=16 index=0 hr=0'
        self.assertFalse(assess(head+row,SOURCE)['copied_event_pose_observations'])
        with self.assertRaises(ValueError):assess(head+row.replace('hr=0','hr=-1'),SOURCE)
    def test_foreign_duplicate_unknown_and_truncated_receipts_reject(self):
        text=fixture();rows=text.splitlines()
        for row in rows:
            if 'Lab idle stream' not in row:continue
            with self.subTest(row=row),self.assertRaises(ValueError):assess(text+'\n'+row,SOURCE)
            with self.subTest(missing=row),self.assertRaises(ValueError):assess('\n'.join(x for x in rows if x!=row),SOURCE)
        for bad in (text.replace('streamProbe request=100','streamProbe request=101'),text.replace('streamProbe','streamUnknown'),
                    text.replace('chunk=0','chunk=1'),text.replace('words=2','words=513'),
                    text.replace('declarationObject=5','declarationObject=0'),text.replace('streamInput '+BASE+' phase=0 index=7','streamInput '+BASE+' phase=0 index=9'),
                    text.replace('values=fffe0101,0000ffff','values=fffe0101'),text.replace('caps=2 declaration=6 declarationObject','caps=257 declaration=6 declarationObject')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_changed_inputs_cannot_claim_same(self):
        text=fixture()
        for a,b in [('phase=1 caps=2 declaration=6 declarationObject=5','phase=1 caps=2 declaration=6 declarationObject=9'),
                    ('phase=1 caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=6','phase=1 caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=9'),
                    ('phase=1 index=7 object=1 offset=48384','phase=1 index=7 object=1 offset=49256'),
                    ('phase=1 index=8 object=1','phase=1 index=8 object=2'),
                    ('phase=1 index=0 values=00000000','phase=1 index=0 values=3f800000')]:
            with self.subTest(a=a),self.assertRaises(ValueError):assess(text.replace(a,b),SOURCE)
            self.assertFalse(assess(text.replace(a,b).replace('flags=511','flags=447'),SOURCE)['copied_event_pose_observations'])
    def test_original_identity_and_declaration_required(self):
        text=fixture()
        for bad in (text.replace('inputDeclaration '+BASE+' index=3 values=7','inputDeclaration '+BASE+' index=3 values=6'),
                    text.replace('valid=15','valid=11'),text.replace('step=20 index=0 hr=0 valid','step=19 index=0 hr=0 valid'),
                    text.replace('preceding=2','preceding=1'),text.replace('state=63','state=55'),
                    text.replace('matrices=4','matrices=0'),text.replace('streamInput '+BASE+' phase=0 index=0 object=1','streamInput '+BASE+' phase=0 index=0 object=2')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_abort_reentry_retirement_cleanup_are_inconclusive(self):
        for mask in (1,2,4,8,16,31):
            text=fixture().replace('invalidations=0','invalidations='+str(mask))
            with self.assertRaises(ValueError):assess(text,SOURCE)
            self.assertFalse(assess(text.replace('flags=511','flags=255'),SOURCE)['copied_event_pose_observations'])
    def test_forward_return_and_flag_dependencies(self):
        text=fixture()
        for flags in (1,2,4,8,16,32,64,128,510,509,507,503,495,479):
            with self.subTest(flags=flags),self.assertRaises(ValueError):assess(text.replace('flags=511','flags='+str(flags)),SOURCE)
        with self.assertRaises(ValueError):assess(text.replace('forwardResult=0','forwardResult=-1'),SOURCE)
    def test_unused_nonfinite_constant_bits_are_diagnostic_only(self):
        r=assess(fixture().replace('values=00000000,00000000,00000000,00000000','values=7fc00000,7f800000,00000000,00000000'),SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
    def test_attempted_phase_requires_prerequisites_even_without_copied_payload(self):
        text=fixture();start=text.index('Lab idle streamSnapshot '+BASE+' phase=1')
        program=text[text.index('Lab idle streamProgram'):]
        head=text[:start]
        failed='Lab idle streamSnapshot '+BASE+' phase=1 status=2 step=9 index=7 hr=-1\n'
        valid=head.replace('flags=511','flags=287')+failed+program
        self.assertFalse(assess(valid,SOURCE)['copied_event_pose_observations'])
        for bad in (valid.replace('flags=287','flags=285'),
                    valid.replace('flags=287','flags=271').replace('forwardResult=0','forwardResult=-1'),
                    valid.replace('status=2 step=9 index=7 hr=-1','status=3 step=9 index=7 hr=0')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
        interrupted=valid.replace('flags=287','flags=31').replace('invalidations=0','invalidations=2').replace('status=2 step=9 index=7 hr=-1','status=3 step=9 index=7 hr=0')
        self.assertFalse(assess(interrupted,SOURCE)['copied_event_pose_observations'])
    def test_exact_original_palette_history_and_bidirectional_equality(self):
        text=fixture()
        for bad in (text.replace('contributors=1','contributors=0'),text.replace('state=63','state=8'),
                    text.replace('callbacks=63','callbacks=0'),text.replace('owner=1','owner=0'),
                    text.replace('rawGripValid=1','rawGripValid=0'),text.replace('flags=511','flags=447')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
        legitimate=text.replace('rawGripValid=1','rawGripValid=0').replace('state=63','state=47')
        self.assertFalse(assess(legitimate,SOURCE)['copied_event_pose_observations'])

if __name__=='__main__':unittest.main()
