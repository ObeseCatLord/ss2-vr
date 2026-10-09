"""Strict bounded API metadata; never content, GPU visibility or a grasp proof."""
from idle_stream_evidence import number,signed
BASE={'request','eye','hand'}

def values(raw,count,signed_indices=()):
    parts=raw.split(',')
    if len(parts)!=count:raise ValueError('Submission vector width mismatch')
    return [signed(v) if i in signed_indices else number(v) for i,v in enumerate(parts)]

def consume(record,kind,f):
    if kind not in ('submissionSummary','submissionRow','submissionMetadata'):return False
    if record is None or not BASE.issubset(f) or any(number(f[k],(1<<64)-1 if k=='request' else 0xffffffff)!=record[k] for k in BASE):
        raise ValueError('Foreign submission metadata')
    if kind=='submissionSummary':
        if set(f)!=BASE|{'attempts','count','overflow','outerReturned','limit'} or 'submissions' in record:
            raise ValueError('Submission summary schema/duplicate')
        p={k:number(f[k]) for k in ('attempts','count','overflow','outerReturned','limit')}
        if p['limit']!=64 or p['count']!=min(p['attempts'],64) or p['overflow']!=int(p['attempts']>64) or p['outerReturned']!=1:
            raise ValueError('Submission budget/outer-return mismatch')
        p.update(rows={},coverage='qualified-bookended-indexed-api-calls-only',gpu_visibility_verified=False,
                 vertex_content_verified=False,hand_resource_verified=False,alignment_accepted=False)
        record['submissions']=p;return True
    p=record.get('submissions')
    if p is None:raise ValueError('Submission data without summary')
    index=number(f.get('index',''),63)
    if index>=p['count']:raise ValueError('Submission row outside reserved attempts')
    if kind=='submissionRow':
        if set(f)!=BASE|{'index','ordinal','status','hr','flags','draw'} or index in p['rows']:
            raise ValueError('Submission row schema/duplicate')
        row={k:number(f[k],15 if k=='flags' else 10 if k=='status' else 0xffffffff) for k in ('ordinal','status','flags')}
        row.update(hresult=signed(f['hr']),draw=values(f['draw'],6,(1,)),metadata=None)
        if row['ordinal']!=index+1 or not row['status'] or (not row['flags']&4 and row['hresult']):
            raise ValueError('Submission ordinal/completion mismatch')
        if row['flags']&8 and row['flags']&3!=3:
            raise ValueError('Submission match without both samples')
        if row['flags']&2 and not row['flags']&4:
            raise ValueError('Post sample without original return')
        status=row['status'];flags=row['flags'];hr=row['hresult']
        if status==1 and (not flags&4 or hr<0 or flags&1) or \
           status==2 and (flags!=5 or hr<0) or status==3 and (flags!=7 or hr<0) or \
           status==9 and (flags&14 or hr):
            raise ValueError('Submission unknown/mismatch/no-forward predicates disagree')
        if row['status']==8 and (row['flags']!=15 or row['hresult']<0) or row['status']==4 and (not row['flags']&4 or row['hresult']>=0):
            raise ValueError('Submission status contradicts original result')
        p['rows'][index]=row;return True
    if set(f)!=BASE|{'index','keys','root','render','layout'}:
        raise ValueError('Submission metadata schema')
    row=p['rows'].get(index)
    if row is None or row['status']!=8 or row['metadata'] is not None:
        raise ValueError('Unqualified/duplicate submission payload')
    keys=values(f['keys'],9,(8,));layout=values(f['layout'],14,(0,1))
    if any(layout[i]>255 for i in (3,4,6,7,9,10,12,13)):
        raise ValueError('Submission channel byte width exceeded')
    if any(not keys[i] for i in (0,1,2,4,5)) or keys[2]>=2048 or keys[3]>=8192 or not 0<=keys[8]<8192:
        raise ValueError('Submission native membership keys outside bounds')
    row['metadata']={'keys':keys,'root':values(f['root'],3,(2,)),
                     'render':values(f['render'],3,(2,)),'layout':layout}
    return True

def validate(record):
    p=record.get('submissions')
    if p is None:return
    if set(p['rows'])!=set(range(p['count'])):raise ValueError('Truncated submission rows')
    for row in p['rows'].values():
        if (row['status']==8)!=(row['metadata'] is not None):raise ValueError('Missing/unqualified submission payload')
