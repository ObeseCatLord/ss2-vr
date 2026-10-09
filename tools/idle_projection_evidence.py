"""Raw native producer bookends. These never replace the independent reference."""
BASE={'request','eye','hand'}
WIDTHS={'model':12,'view':12,'projection':16,'cachedVP':16,'cachedMVP':16}

def consume(record,kind,f,integer,hexwords):
    if not BASE.issubset(f) or any(integer(f[k])!=record[k] for k in BASE):
        raise ValueError('Foreign projection producer record')
    if kind=='projectionSummary':
        if set(f)!=BASE|{'configured','count','invalidations','blocked','pending'} or 'projection_probe' in record:
            raise ValueError('Projection summary schema/duplicate')
        p={k:integer(f[k],0,8 if k=='count' else 0xffffffff if k=='invalidations' else 1)
           for k in ('configured','count','invalidations','blocked','pending')}
        if p['pending'] or bool(p['blocked'])!=bool(p['invalidations']) or p['count'] and not p['configured']:
            raise ValueError('Unretired/inconsistent projection owner')
        p.update(pairs={},diagnostic_only=True,reference_replaced=False,alignment_accepted=False)
        record['projection_probe']=p;return
    p=record.get('projection_probe')
    if p is None:raise ValueError('Projection payload before summary')
    sequence=integer(f.get('sequence','0'),1,p['count'])
    if kind=='projectionPair':
        names={'sequence','source','complete','controlBefore','controlAfter','flagsBefore','flagsAfter',
               'modelBefore','modelAfter','drawBefore','drawAfter','cleanupCertified','outerCurrent'}
        if set(f)!=BASE|names or sequence in p['pairs']:raise ValueError('Projection pair schema/duplicate')
        v={k:integer(f[k],0,65535 if k.startswith('control') else 0xffffffff) for k in names}
        if v['source'] not in (1,2) or v['complete']!=1 or v['cleanupCertified'] or v['outerCurrent'] or \
           v['controlBefore']!=v['controlAfter'] or v['flagsAfter']!=(v['flagsBefore']|6) or \
           not v['modelBefore'] or v['modelBefore']!=v['modelAfter'] or \
           not v['drawBefore'] or v['drawBefore']!=v['drawAfter']:
            raise ValueError('Unsupported native producer bookend claim')
        v.update(data={},diagnostic_only=True,reference_replaced=False,
                 earlier_vp_producer_verified=False,earlier_mvp_producer_verified=False,
                 vp_computed=not bool(v['flagsBefore']&2),mvp_computed=not bool(v['flagsBefore']&4),
                 precision_bits=(v['controlBefore']>>8)&3,rounding_bits=(v['controlBefore']>>10)&3)
        p['pairs'][sequence]=v;return
    if kind!='projectionData' or set(f)!=BASE|{'sequence','phase','kind','values'}:
        raise ValueError('Unknown projection producer payload')
    v=p['pairs'].get(sequence)
    if v is None:raise ValueError('Projection data before pair')
    phase=integer(f['phase'],0,1);which=f['kind'];key=str(phase)+':'+which
    if which not in WIDTHS or key in v['data']:raise ValueError('Unknown/duplicate projection operands')
    v['data'][key]=hexwords(f['values'],WIDTHS[which])

def validate(record):
    p=record.get('projection_probe')
    if p is None:return
    if set(p['pairs'])!=set(range(1,p['count']+1)):raise ValueError('Missing native producer pairs')
    wanted={str(phase)+':'+k for phase in range(2) for k in WIDTHS}
    for v in p['pairs'].values():
        if set(v['data'])!=wanted:raise ValueError('Truncated native producer operands')
        if any(v['data']['0:'+k]!=v['data']['1:'+k] for k in ('model','view','projection')):
            raise ValueError('Changed independent native producer inputs')
        for flag,name in ((2,'cachedVP'),(4,'cachedMVP')):
            if v['flagsBefore']&flag and v['data']['0:'+name]!=v['data']['1:'+name]:
                raise ValueError('Changed cache words on native reuse')

def validate_association(record,g):
    a=g.get('projection_association')
    if a is None:return
    p=record.get('projection_probe',{}).get('pairs',{}).get(a['sequence'])
    if p is None or 'factors' not in g:raise ValueError('Association without producer/factor evidence')
    if a['model_address']!=p['modelAfter'] or a['draw_address']!=p['drawAfter']:
        raise ValueError('Wrong native producer membership keys')
    for native,factor in (('model','factorModel'),('view','factorView'),('projection','factorProjection')):
        if p['data']['1:'+native]!=g['data'][factor+':0']:
            raise ValueError('Producer inputs disagree with independent draw factors')
    if g['constants']<5 or p['data']['1:cachedMVP']!=sum((g['data']['constant:'+str(i)] for i in range(1,5)),[]):
        raise ValueError('Native cache output disagrees with actual draw constants')
    a.update(diagnostic_only=True,reference_replaced=False,alignment_accepted=False,
             cleanup_certified=False,outer_current=False)
