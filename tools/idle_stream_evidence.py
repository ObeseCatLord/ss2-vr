"""Strict passive selected-stream receipts; never admit geometry or infer stream roles."""
import re

OBSERVED=[[0,0,2,0,5,0],[2,0,1,0,5,2],[3,0,1,0,5,3],
          [7,0,8,0,5,7],[8,0,8,0,5,8],[255,0,17,0,0,0]]
NO_UV56=[[0,0,2,0,5,0],[1,0,2,0,5,1],[5,0,8,0,5,5],
         [6,0,8,0,5,6],[255,0,17,0,0,0]]
# Passive observation only; extra inputs remain unobserved/unknown. Never
# include this declaration in declaration_layout's production grammar.
OBSERVED_MULTI_UV=OBSERVED[:3]+[[4,0,1,0,5,4],[5,0,1,0,5,5]]+OBSERVED[3:]

def diagnostic_stream_numbers(failure):
    count=failure.get('declaration',0)
    if count not in (5,6,8):raise ValueError('Unsupported passive declaration count')
    rows=[failure.get('declaration_rows',{}).get(i) for i in range(count)]
    if rows==OBSERVED or rows==OBSERVED_MULTI_UV:return (0,7,8)
    if rows==NO_UV56:return (0,5,6)
    raise ValueError('Unsupported passive original declaration')

BASE={'request','eye','hand'}

def declaration_layout(rows):
    if rows==OBSERVED:return 1,True
    if rows==NO_UV56:return 2,True
    if any(e[0] in (7,8) or (e[4]==5 and e[5] in (7,8)) for e in rows):
        raise ValueError('Ambiguous/mixed observed input family')
    seen=set();active=set()
    for i,e in enumerate(rows):
        stream,offset,kind,method,usage,index=e
        if stream>65535 or offset>65535 or any(x>255 for x in e[2:]):raise ValueError('Declaration field exceeds native width')
        if stream==255:
            if i!=len(rows)-1 or e!=[255,0,17,0,0,0]:raise ValueError('Invalid declaration end')
            break
        if stream>15:raise ValueError('Unsupported declaration stream')
        if usage==5 and index in (0,3,5,6) and stream!=index:raise ValueError('Aliased declaration semantic')
        if stream not in (0,3,5,6):continue
        if stream in seen:raise ValueError('Duplicate declaration input')
        seen.add(stream)
        if kind==17:
            if stream!=6:raise ValueError('Missing required declaration input')
            continue
        if offset or method or usage!=5 or index!=stream or kind!={0:2,3:1,5:8,6:8}[stream]:
            raise ValueError('Unsupported declaration input type')
        active.add(stream)
    else:raise ValueError('Missing declaration end')
    if not {0,3,5}.issubset(active):raise ValueError('Missing required declaration stream')
    return 0,6 in active


def number(value,maximum=0xffffffff):
    if not re.fullmatch(r'\d+',value) or not 0<=int(value)<=maximum:raise ValueError('Invalid stream diagnostic integer')
    return int(value)

def signed(value):
    if not re.fullmatch(r'-?\d+',value) or not -(1<<31)<=int(value)<(1<<31):raise ValueError('Invalid stream HRESULT')
    return int(value)

def words(value,count):
    v=value.split(',')
    if len(v)!=count or any(not re.fullmatch('[0-9a-f]{8}',x) for x in v):raise ValueError('Incomplete stream raw words')
    return [int(x,16) for x in v]

def schema(f,extra):
    if set(f)!=BASE|set(extra):raise ValueError('Stream diagnostic field mismatch')

def consume(record,kind,f):
    if kind not in ('streamProbe','streamSnapshot','streamBinding','streamInput','streamDeclaration','streamConstant','streamProgram') or not BASE.issubset(f):
        raise ValueError('Unknown/incomplete stream diagnostic kind')
    failure=record.get('input_failure',{})
    if record.get('stage')!=3 or record.get('rejection',{}).get('reason')!=32 or \
            failure.get('step')!=20 or failure.get('valid')!=15:
        raise ValueError('Passive stream receipt without rejected full step20 input')
    selected=diagnostic_stream_numbers(failure)
    if any(number(f[k],(1<<64)-1 if k=='request' else 0xffffffff)!=record[k] for k in BASE):
        raise ValueError('Foreign/interleaved stream receipt')
    if kind=='streamProbe':
        schema(f,{'attempts','flags','words','invalidations','forwardResult'})
        if 'stream_probe' in record:raise ValueError('Duplicate stream probe')
        record['stream_probe']={k:number(f[k],m) for k,m in [('attempts',2),('flags',511),('words',512),('invalidations',31)]}
        record['stream_probe'].update(forward_result=signed(f['forwardResult']),snapshots={},program={})
        return
    p=record.get('stream_probe')
    if p is None:raise ValueError('Stream payload before header')
    if kind=='streamProgram':
        schema(f,{'chunk','values'})
        chunk=number(f['chunk'],511)
        if chunk%32 or chunk>=p['words'] or chunk in p['program']:raise ValueError('Invalid/duplicate stream program chunk')
        p['program'][chunk]=words(f['values'],min(32,p['words']-chunk));return
    phase=number(f['phase'],1)
    if phase>=p['attempts']:raise ValueError('Stream snapshot beyond attempted phases')
    if kind=='streamSnapshot':
        schema(f,{'phase','status','step','index','hr'})
        if phase in p['snapshots']:raise ValueError('Duplicate stream snapshot')
        status=number(f['status'],3);step=number(f['step'],21);index=number(f['index'],8);hr=signed(f['hr'])
        if status not in (1,2,3) or (status==1 and (step or index or hr)) or \
                (status!=1 and not step) or (phase and step>18) or \
                (step in (9,10,11,12) and index not in selected) or \
                (step not in (9,10,11,12) and index) or (status==3 and hr):
            raise ValueError('Contradictory stream snapshot status')
        api_failures={1,3,4,6,9,11,13,16,19}
        predicates={2,5,7,10,14,17,20}
        if status==2 and ((step in api_failures and hr>=0) or (step in predicates and hr<0)):
            raise ValueError('Stream failure HRESULT qualification mismatch')
        p['snapshots'][phase]={'status':status,'step':step,'index':index,'hr':hr,
                                'binding':None,'streams':{},'declaration':{},'constants':{}}
        return
    s=p['snapshots'].get(phase)
    if s is None or s['status']!=1:raise ValueError('Unqualified partial stream payload')
    if kind=='streamBinding':
        schema(f,{'phase','caps','declaration','declarationObject','indexObject','shaderObject'})
        if s['binding'] is not None:raise ValueError('Duplicate stream binding')
        v={k:number(f[k]) for k in ('caps','declaration','declarationObject','indexObject','shaderObject')}
        if not 1<=v['caps']<=256 or not 1<=v['declaration']<=65 or any(not v[k] for k in ('declarationObject','indexObject','shaderObject')):
            raise ValueError('Missing bounded stream binding outputs')
        s['binding']=v;return
    if s['binding'] is None:raise ValueError('Stream arrays before bounded binding')
    index=number(f['index'])
    if kind=='streamInput':
        schema(f,{'phase','index','object','offset','stride','frequency'})
        if index not in selected or index in s['streams']:raise ValueError('Invalid/duplicate actual stream')
        v={k:number(f[k]) for k in ('object','offset','stride','frequency')}
        if not v['object']:raise ValueError('Null copied stream identity')
        s['streams'][index]=v
    elif kind=='streamDeclaration':
        schema(f,{'phase','index','values'})
        if index>=s['binding']['declaration'] or index in s['declaration']:raise ValueError('Invalid/duplicate stream declaration')
        v=[number(x) for x in f['values'].split(',')]
        if len(v)!=6 or any(x>m for x,m in zip(v,[65535,65535,255,255,255,255])):raise ValueError('Stream declaration width mismatch')
        s['declaration'][index]=v
    elif kind=='streamConstant':
        schema(f,{'phase','index','values'})
        if index>=s['binding']['caps'] or index in s['constants']:raise ValueError('Invalid/duplicate stream constant')
        s['constants'][index]=words(f['values'],4)
    else:raise ValueError('Unknown stream diagnostic kind')

def validate(record,*,retained_validated=False):
    p=record.get('stream_probe')
    if p is None:return
    # The reader passes this only after complete retained-companion qualification
    # and the shared strict geometry validator. A JSON flag cannot enable it.
    failure=record['input_failure'];reason=record['rejection']
    selected=diagnostic_stream_numbers(failure)
    original_rows=[failure['declaration_rows'][i] for i in range(failure['declaration'])]
    if reason['preceding']!=2 or reason['checks'] or reason['state']!=(47|record['rawGripValid']*16) or \
            reason['callbacks']!=63 or not 1<=record['contributors']<=16 or \
            not 1<=record['matrices']<=64 or (record['draws'] and not retained_validated) or \
            any(not record[k] for k in ('request','input','owner','weapon','model','generation','cfg','file')):
        raise ValueError('Passive stream selector not qualified by original observation')
    if set(p['snapshots'])!=set(range(p['attempts'])) or set(p['program'])!=set(range(0,p['words'],32)):
        raise ValueError('Truncated passive stream receipt')
    for s in p['snapshots'].values():
        if s['status']==1:
            b=s['binding']
            if b is None or set(s['streams'])!=set(selected) or set(s['declaration'])!=set(range(b['declaration'])) or \
                    set(s['constants'])!=set(range(b['caps'])):raise ValueError('Truncated copied stream snapshot')
    flags=p['flags'];before=p['snapshots'].get(0);after=p['snapshots'].get(1)
    if p['attempts']==2 and flags&18!=18:
        raise ValueError('Post-forward sampling without original current/success prerequisites')
    if any(s['status']==3 for s in p['snapshots'].values()) and (not p['invalidations']&2 or flags&256):
        raise ValueError('Interrupted sampling without abnormal cleanup')
    if bool(flags&1)!=(before is not None and before['status']==1) or \
            bool(flags&32)!=(after is not None and after['status']==1):raise ValueError('Snapshot copy flags disagree')
    if bool(p['words'])!=bool(flags&1) or (p['words'] and p['words']<2):raise ValueError('Program copy qualification mismatch')
    if flags&2:
        if not flags&1 or before['binding']['caps']!=failure['caps'] or \
                [before['declaration'][i] for i in range(before['binding']['declaration'])]!=original_rows or \
                before['streams'][0]!=failure['streams'][0]:raise ValueError('Before-current evidence contradicts original inputs')
    if flags&2 and selected==(0,5,6) and (
            before['streams'][5]!=failure['streams'][1] or before['streams'][6]!=failure['streams'][3]):
        raise ValueError('Before-current selected 5/6 bindings contradict original inputs')
    if flags&8 and not flags&4 or flags&16 and flags&12!=12 or flags&32 and flags&18!=18 or \
            flags&128 and not flags&32 or flags&64 and flags&35!=35:
        raise ValueError('Invalid passive draw flag dependencies')
    if not flags&8 and p['forward_result'] or flags&8 and bool(flags&16)!=(p['forward_result']>=0):
        raise ValueError('Unqualified forward HRESULT')
    equal=before is not None and after is not None and before['status']==1 and after['status']==1 and \
        all(before[k]==after[k] for k in ('binding','streams','declaration','constants'))
    if bool(flags&64)!=bool(equal):raise ValueError('Stream equality flag contradicts copied inputs')
    if flags&256 and p['invalidations']:raise ValueError('Invalidated passive cleanup marked current')
    p['evidence_class']='passive-rejected-draw-observation'
    p['geometry_admitted']=False;p['roles_inferred']=False;p['alignment_accepted']=False
