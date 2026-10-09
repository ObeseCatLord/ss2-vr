"""Independent first-material cold projection reference, for offline copies only.

The raw native outputs corroborate a calculation from M/V/P; they never seed it.
Unknown earlier cache producers, other arithmetic modes and other material paths
remain unsupported. No native state or admission/tolerance policy is changed.
"""
from fractions import Fraction
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

class UnsupportedNativeArithmetic(ValueError):
    """Well-formed diagnostic outside the independently reproduced arithmetic."""

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

def native_reference(record,geometry):
    """Return a supported cold association, or None; malformed evidence rejects.

    Signed-zero or other raw-word disagreement rejects the companion rather than
    claiming a broader arithmetic implementation. Cached words are comparisons
    only. A matching reference does not promote rejected history or prove grasp.
    """
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
