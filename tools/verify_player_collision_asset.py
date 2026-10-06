#!/usr/bin/env python3
"""Fingerprint-bound audit of owned player collision templates; no asset export.

This is not a general CTSEMETA loader, a live body admission or a physics test.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ASSET='Content/SeriousSam2/Models/Player/SeriousSam/Mechanisms/Player.mch'
PIN='e112046f8ac9fc7327fafdb29892ab50649aac0cd503f7a3e82e0cb30f1b5772'


def require(value,message):
    if not value: raise ValueError(message)


def verify(game):
    data=(game/ASSET).read_bytes()
    require(len(data)==10056 and hashlib.sha256(data).hexdigest()==PIN,'Unsupported player mechanism asset')
    def u(at):return struct.unpack_from('<I',data,at)[0]
    require(data[0x12b:0x12f]==b'IDNT' and u(0x12f)==26,'Name table changed')
    names={};at=0x133
    for _ in range(26):
        key,length=struct.unpack_from('<II',data,at);at+=8
        require(key not in names and length<=128 and at+length<=len(data),'Invalid name entry')
        names[key]=data[at:at+length].decode('ascii');at+=length
    for offset,index,name in [(0x578,11,b'CMechanismTemplate'),(0xa61,33,b'CHibridBodyTemplate'),
                              (0xace,35,b'CPrimitiveHullTemplate'),(0xb16,36,b'CPrimitiveDesc')]:
        require(data[offset:offset+4]==b'DTTY' and u(offset+4)==index and
                data[offset+12:offset+12+u(offset+8)]==name,'Serialized type changed')
    require(data[0xf3d:0xf41]==b'OBJS','Object framing changed')
    result={}
    profiles=[('Default',2,0xfa9,9,0x1b48,[11],[0x1b80]),
              ('Swimming',5,0x18d0,70,0x254c,[72,73],[0x2584,0x25c0]),
              ('Crouch',7,0x1a20,78,0x2688,[80],[0x26c0])]
    for name,obj,offset,body,bodyOffset,hulls,hullOffsets in profiles:
        require((u(offset),u(offset+4))==(obj,11) and names[u(offset+8)]==name,'Profile identity changed')
        require(u(offset+0x30)==body and (u(bodyOffset),u(bodyOffset+4))==(body,33),
                'Profile hybrid body binding changed')
        shapeStart=offset+0x38
        require(data[shapeStart:shapeStart+4]==b'STAR' and u(shapeStart+4)==len(hulls),
                'Profile shape count changed')
        shapes=[]
        for i,(hull,hullOffset) in enumerate(zip(hulls,hullOffsets)):
            require(u(shapeStart+8+i*20+12)==hull,'Shape reference changed')
            require((u(hullOffset),u(hullOffset+4))==(hull,35),'Primitive object changed')
            pose=struct.unpack_from('<7f',data,hullOffset+8)
            kind,width,height,depth=struct.unpack_from('<I3f',data,hullOffset+0x28)
            require(kind==2 and width>0 and height>=width,'Profile is not the inspected capsule')
            shapes.append({'pose_xyzw_xyz':pose,'kind':kind,'width':width,'height':height,'depth':depth})
        children=shapeStart+8+20*len(hulls)
        require(data[children:children+4]==b'STAR' and u(children+4)==0,'Unexpected child parts')
        result[name]={'hybrid_root':True,'child_parts':0,'shapes':shapes}
    return {'asset_sha256':PIN,'runtime_executed':False,'live_dimensions_hardcoded':False,'profiles':result}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    print(json.dumps(verify(parser.parse_args().game),indent=2))
