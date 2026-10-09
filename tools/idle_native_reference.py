"""Independent copied-transform references, for offline copies only.

The raw native outputs corroborate a calculation from M/V/P; they never seed it.
The narrow uploaded-transform case corroborates a fixed independent calculation
without asserting an earlier cache producer or observed arithmetic mode.
No native state or admission/tolerance policy is changed.
"""
from fractions import Fraction
import hashlib
import math
import struct
from idle_projection_evidence import validate, validate_association

# Conceptual dot-product addition order of the pinned first native material.
# Its affine fourth row is (0,0,0,1). Every multiply/add rounds to 24 significant
# bits; each Q/R output is then stored as binary32 before the next matrix stage.
VP_ORDER=((1,0,2),(0,1,2),(0,1,2),(0,1,2),
          (0,1,2),(1,0,2),(1,0,2),(1,0,2),
          (1,0,2),(1,2,0),(1,2,0),(1,2,0),
          (0,1,2),(1,0,2),(1,0,2),(1,0,2))
MVP_ORDER=((2,0,1),(0,2,1),(0,2,1),(0,2,1),
           (2,0,1),(2,0,1),(0,2,1),(0,2,1),
           (2,1,0),(2,0,1),(0,2,1),(0,2,1),
           (2,0,1),(2,0,1),(0,2,1),(0,2,1))

# Pinned Poly Bump instruction order, selected by the own source2 bookends,
# never by which arithmetic happens to agree with an uploaded result.
POLY_VP_ORDER=((2,1,0),(2,0,1),(2,0,1),(2,0,1),
               (2,1,0),(2,0,1),(2,0,1),(2,0,1),
               (2,0,1),(2,0,1),(2,0,1),(2,0,1),
               (2,1,0),(2,0,1),(2,0,1),(2,0,1))
POLY_MVP_ORDER=((1,0,2),(1,0,2),(1,0,2),(1,0,2),
                (0,1,2),(1,2,0),(1,2,0),(1,2,0),
                (1,0,2),(1,2,0),(1,2,0),(1,2,0),
                (0,1,2),(1,2,0),(1,2,0),(1,2,0))

class UnsupportedNativeArithmetic(ValueError):
    """Well-formed diagnostic outside the independently reproduced arithmetic."""

class UnsupportedUploadedTransform(UnsupportedNativeArithmetic):
    """Known program, but its own copied transforms are not corroborated."""
    def __init__(self,reason,api_association=None):
        super().__init__(reason)
        self.api_association=api_association

def uploaded_api_association(record,geometry,index):
    """Join copied geometry to its own API row without choosing by arithmetic."""
    def unknown(reason):raise UnsupportedUploadedTransform(reason)
    if record.get('schema')!=4 or record.get('copyLayout')!=1 or record.get('stage')!=4:
        unknown('uploaded-transform-owner-incomplete')
    p=record.get('submissions')
    if p is None or p['overflow'] or not p['outerReturned']:
        unknown('uploaded-transform-api-coverage-unknown')
    def matches(g,row):
        m=row['metadata'];d=g['data']
        keys=[g[k] for k in ('modelRecord','drawRecord','surface','instance','surfaceName','boneName','bone')]
        return m is not None and m['keys'][2:]==keys and \
            m['root']==[record[k] for k in ('cfg','file','resource')] and \
            m['render']==[g[k] for k in ('cfg','file','resource')] and \
            m['layout']==d['layout:0'] and row['draw']==d['draw:0']
    def qualified(row,position):
        return row is not None and (row['ordinal'],row['status'],row['flags'])==(position+1,8,15) and row['hresult']>=0
    if p['attempts']==record['draws']:
        # Preserve historical ordinal behavior, including repeated identities.
        row=p['rows'].get(index,p['rows'].get(str(index)))
        if not qualified(row,index):unknown('uploaded-transform-api-bookends-missing')
        if not matches(geometry,row):unknown('uploaded-transform-copy-identity-disagreement')
        return {'api_index':index,'api_ordinal':index+1,'association_kind':'equal-count-ordinal',
                'api_geometry_coverage_complete':True}
    if not 0<record['draws']<p['attempts']<=64 or p['count']!=p['attempts']:
        unknown('uploaded-transform-api-coverage-unknown')
    def ordered(values,count):
        if set(values)!=set(range(count)) and set(values)!=set(map(str,range(count))):
            unknown('uploaded-transform-api-table-incomplete')
        return [values.get(i,values.get(str(i))) for i in range(count)]
    rows=ordered(p['rows'],p['attempts'])
    copies=ordered(record['geometry'],record['draws'])
    if not 0<=index<len(copies) or copies[index]!=geometry:
        unknown('uploaded-transform-copy-identity-disagreement')
    for position,row in enumerate(rows):
        if not qualified(row,position) or row['metadata'] is None:
            unknown('uploaded-transform-api-bookends-missing')
        if row['metadata']['root']!=[record[k] for k in ('cfg','file','resource')]:
            unknown('uploaded-transform-api-foreign-root')
    mapping=[]
    for copy in copies:
        candidates=[i for i,row in enumerate(rows) if matches(copy,row)]
        if len(candidates)!=1:unknown('uploaded-transform-api-association-not-unique')
        mapping.append(candidates[0])
    if any(a>=b for a,b in zip(mapping,mapping[1:])):
        unknown('uploaded-transform-api-association-not-ordered-injective')
    selected=mapping[index]
    return {'api_index':selected,'api_ordinal':selected+1,'association_kind':'surplus-unique-identity',
            'api_geometry_coverage_complete':False}

UPLOADED_PROGRAM_HASH='6118c85d5b3613bb50c9b82c62991e0246b012e55be02254e6e589e07d364497'
UPLOADED_DECLARATION=((0,0,2,0,5,0),(1,0,2,0,5,1),(2,0,3,0,5,2),
                      (3,0,1,0,5,3),(4,0,1,0,5,4),(5,0,8,0,5,5),
                      (6,0,8,0,5,6),(255,0,17,0,0,0))

def finite_words(words, count):
    if len(words)!=count or any(type(v) is not int or not 0<=v<=0xffffffff for v in words):
        raise ValueError('Independent matrix word inventory')
    values=struct.unpack('<'+str(count)+'f',struct.pack('<'+str(count)+'I',*words))
    if not all(math.isfinite(v) for v in values):raise ValueError('Nonfinite independent matrix')
    return [Fraction(v) for v in values]

def round24(value):
    if not value:return value
    sign=-1 if value<0 else 1
    value=abs(value)
    exponent=value.numerator.bit_length()-value.denominator.bit_length()
    if value<Fraction(2)**exponent:exponent-=1
    step=Fraction(2)**(exponent-23)
    numerator,denominator=(value/step).as_integer_ratio()
    whole,remainder=divmod(numerator,denominator)
    if 2*remainder>denominator or (2*remainder==denominator and whole&1):whole+=1
    return sign*whole*step

def store32(value):
    try:raw=struct.pack('<f',float(value))
    except (OverflowError,struct.error) as error:raise UnsupportedNativeArithmetic('unsupported-matrix-store') from error
    result=struct.unpack('<f',raw)[0]
    if not math.isfinite(result):raise UnsupportedNativeArithmetic('nonfinite-matrix-store')
    return Fraction(result),struct.unpack('<I',raw)[0]

def _multiply_affine(left,right,order):
    values=[];words=[]
    for row in range(4):
        for col in range(4):
            terms=[round24(left[row*4+k]*right[k*4+col]) for k in range(3)]
            a,b,c=order[row*4+col]
            total=round24(round24(terms[a]+terms[b])+terms[c])
            total=round24(total+(left[row*4+3] if col==3 else Fraction(0)))
            value,word=store32(total);values.append(value);words.append(word)
    return values,words

def first_material_matrices(model,view,projection):
    """Fixed PC24/nearest calculation from independent binary32 inputs."""
    M=finite_words(model,12);V=finite_words(view,12);P=finite_words(projection,16)
    Q,qwords=_multiply_affine(P,V,VP_ORDER)
    _,rwords=_multiply_affine(Q,M,MVP_ORDER)
    return qwords,rwords

def poly_bump_matrices(model,view,projection):
    """Bounded PC24/nearest reference; raw signed-zero mismatches reject."""
    M=finite_words(model,12);V=finite_words(view,12);P=finite_words(projection,16)
    Q,qwords=_multiply_affine(P,V,POLY_VP_ORDER)
    _,rwords=_multiply_affine(Q,M,POLY_MVP_ORDER)
    return qwords,rwords

def uploaded_transform_reference(record,geometry,channels,index):
    """Compute from this copy's M/V/P/L; uploads corroborate, never seed.

    Only the observed schema4 program and owned API association are supported.
    The consumer's mode is distinct from an earlier cache producer's mode.
    The caller has already assessed the log and uniquely matched its channels.
    """
    if record.get('schema')!=4:return None
    d=geometry['data']
    if geometry['words']!=320:return None
    program=[w for i in range(0,320,32) for w in d['program:'+str(i)]]
    if hashlib.sha256(struct.pack('<320I',*program)).hexdigest()!=UPLOADED_PROGRAM_HASH:return None
    association=None
    def unknown(reason):raise UnsupportedUploadedTransform(reason,association)
    if record['stage']!=4 or record['copyLayout']!=1:unknown('uploaded-transform-owner-incomplete')
    declaration=tuple(tuple(d['declaration:'+str(i)]) for i in range(geometry['declaration']))
    if declaration!=UPLOADED_DECLARATION or geometry['input_layout']!=0:
        unknown('uploaded-transform-declaration-unsupported')
    f=geometry.get('factors')
    if f is None or (f['bookends'],f['post_original_return'],f['cleanup_certified'],f['outer_current'])!=(3,True,False,False):
        unknown('uploaded-transform-factor-bookends-missing')
    association=uploaded_api_association(record,geometry,index)
    vertices=d['layout:0'][0]
    if len(channels['weights'])!=4*vertices or len(channels['local_indices'])!=4*vertices:
        raise ValueError('Uploaded transform influence byte inventory')
    if channels['weights']!=bytes((255,0,0,0))*vertices or channels['local_indices']!=bytes(4*vertices):
        unknown('uploaded-transform-influences-unsupported')
    if geometry['constants']<24:unknown('uploaded-transform-constants-incomplete')
    model=d['factorModel:0'];view=d['factorView:0'];projection=d['factorProjection:0'];local=d['factorLocal:0']
    finite_words(local,12)
    arithmetic=first_material_matrices;arithmetic_source='first-material-conditional-pc24-nearest'
    consumer_control=None;pair=None
    a=geometry.get('projection_association')
    if a is None and (record.get('nativeId')==13 or any(
            p['source']==2 for p in record.get('projection_probe',{}).get('pairs',{}).values())):
        unknown('uploaded-transform-poly-own-association-missing')
    if a is not None:
        validate(record);validate_association(record,geometry)
        pair=record['projection_probe']['pairs'][a['sequence']]
        if record.get('nativeId')==13 and pair['source']!=2:
            unknown('uploaded-transform-poly-own-source-missing')
        if pair['source']==2:
            if (pair['controlBefore'],pair['controlAfter'])!=(127,127):
                unknown('uploaded-transform-poly-consumer-mode-unsupported')
            row=record['submissions']['rows'].get(association['api_index'],
                record['submissions']['rows'].get(str(association['api_index'])))
            if row['metadata']['keys'][:2]!=[pair['modelAfter'],pair['drawAfter']]:
                unknown('uploaded-transform-poly-api-pair-identity-disagreement')
            arithmetic=poly_bump_matrices
            arithmetic_source='poly-bump-own-consumer-pc24-nearest'
            consumer_control=127
        elif pair['source']!=1:
            unknown('uploaded-transform-projection-source-unsupported')
    try:q,r=arithmetic(model,view,projection)
    except UnsupportedNativeArithmetic as error:
        raise UnsupportedUploadedTransform(str(error),association) from error
    if consumer_control is not None and (q!=pair['data']['1:cachedVP'] or r!=pair['data']['1:cachedMVP']):
        unknown('uploaded-transform-poly-cache-word-disagreement')
    if r!=[v for i in range(1,5) for v in d['constant:'+str(i)]]:
        unknown('uploaded-transform-mvp-word-disagreement')
    if local!=[v for i in range(21,24) for v in d['constant:'+str(i)]]:
        unknown('uploaded-transform-local-word-disagreement')
    return {'kind':'uploaded-transform-corroboration','matrix':r,'local':list(local),'api_association':association,
            'arithmetic_source':arithmetic_source,'own_consumer_control_word':consumer_control,
            'own_consumer_mode_observed':consumer_control is not None,
            'uploads_used_as_inputs':False,'cache_outputs_used_as_inputs':False,
            'native_producer_verified':False,'pc_rc_observed':False,'diagnostic_only':True,
            'positive_grasp_verified':False,'alignment_accepted':False}

def native_reference(record,geometry):
    """Return a supported cold association, or None; malformed evidence rejects.

    Signed-zero or other raw-word disagreement rejects the companion rather than
    claiming a broader arithmetic implementation. Cached words are comparisons
    only. A matching reference does not promote rejected history or prove grasp.
    """
    # ID13's observed program needs its own Poly Bump consumer. A foreign
    # source1 cold pair cannot bypass that guard via cold-reference precedence.
    if record.get('nativeId')==13:return None
    association=geometry.get('projection_association')
    if association is None:return None
    validate(record);validate_association(record,geometry)
    pair=record['projection_probe']['pairs'][association['sequence']]
    if pair['source']!=1 or pair['controlBefore']!=127 or pair['flagsBefore']!=0:
        return None
    d=pair['data']
    q,r=first_material_matrices(d['0:model'],d['0:view'],d['0:projection'])
    if q!=d['1:cachedVP'] or r!=d['1:cachedMVP']:
        raise UnsupportedNativeArithmetic('independent-cold-producer-word-disagreement')
    local=geometry['data']['factorLocal:0'];finite_words(local,12)
    return {'kind':'cold-first-material-pc24-nearest-staged-local',
            'matrix':r,'local':list(local),'producer_sequence':association['sequence'],
            'cache_outputs_used_as_inputs':False,'diagnostic_only':True,
            'positive_grasp_verified':False,'alignment_accepted':False}
