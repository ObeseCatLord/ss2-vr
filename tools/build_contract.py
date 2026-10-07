"""Read compiled package contracts and match them to the current source tree."""
import hashlib
from pathlib import Path
import re
import struct
import pefile

FORMAT = struct.Struct('<16s12I64s')
MAGIC = b'SS2VR_BUILD_V1'.ljust(16, b'\0')
FIELDS = ('schema', 'component', 'ipc_abi', 'multiplayer_wire_version',
          'input_bytes', 'request_bytes', 'ui_bytes', 'slot_bytes', 'shared_bytes',
          'version_major', 'version_minor', 'version_patch')
COMPONENTS = {'game': (1, 0x14c), 'server': (2, 0x14c), 'host': (3, 0x8664)}


def source_contract(root):
    root = Path(root)
    paths = [root / 'CMakeLists.txt']
    for directory in ['src', 'cmake']:
        paths.extend(p for p in (root / directory).rglob('*') if p.is_file())
    fingerprint = hashlib.sha256()
    for path in sorted(paths, key=lambda p: p.relative_to(root).as_posix()):
        if path.is_symlink():
            raise ValueError('Source contract refuses symlink inputs: ' + str(path))
        relative = path.relative_to(root).as_posix()
        if any(c in relative for c in '\n\r;'):
            raise ValueError('Unsupported source-contract filename')
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        fingerprint.update((relative + ':' + digest + '\n').encode('utf-8'))
    version = re.findall(r'project\(ss2vr\s+VERSION\s+(\d+)\.(\d+)\.(\d+)\s',
                         (root / 'CMakeLists.txt').read_text())
    abi = re.findall(r'\bAbi\s*=\s*(\d+)\b', (root / 'src/common/protocol.hpp').read_text())
    wire = re.findall(r'\bWireVersion\s*=\s*(\d+)\b', (root / 'src/common/network.hpp').read_text())
    if len(version) != 1 or len(abi) != 1 or len(wire) != 1:
        raise ValueError('Missing or ambiguous source version/ABI constants')
    return {'source_fingerprint': fingerprint.hexdigest(), 'ipc_abi': int(abi[0]),
            'multiplayer_wire_version': int(wire[0]),
            **dict(zip(FIELDS[-3:], map(int, version[0])))}


def decode_contract(data):
    if len(data) != FORMAT.size:
        raise ValueError('Truncated compiled build contract')
    magic, *values, fingerprint = FORMAT.unpack(data)
    if magic != MAGIC:
        raise ValueError('Unknown compiled build-contract magic')
    result = dict(zip(FIELDS, values))
    if result['schema'] != 1 or result['component'] not in (1, 2, 3):
        raise ValueError('Unsupported build-contract schema/component')
    if any(not result[key] for key in FIELDS[2:9]):
        raise ValueError('Invalid zero version/layout field')
    if re.fullmatch(b'[0-9a-f]{64}', fingerprint) is None:
        raise ValueError('Malformed source fingerprint')
    result['source_fingerprint'] = fingerprint.decode('ascii')
    return result


def read_contract(path, component):
    expected_component, machine = COMPONENTS[component]
    with pefile.PE(str(path), max_symbol_exports=100000) as pe:
        if pe.FILE_HEADER.Machine != machine:
            raise ValueError('Wrong product architecture: ' + str(path))
        exports = getattr(pe, 'DIRECTORY_ENTRY_EXPORT', None)
        matches = [e for e in exports.symbols if e.name == b'ss2vrBuildContract'] if exports else []
        if len(matches) != 1 or matches[0].forwarder:
            raise ValueError('Missing/ambiguous compiled build contract: ' + str(path))
        address = matches[0].address
        section = pe.get_section_by_rva(address)
        if (section is None or section.Characteristics & 0xa0000000 or
                not section.Characteristics & 0x40000000 or
                address < section.VirtualAddress or
                address + FORMAT.size > section.VirtualAddress + section.SizeOfRawData or
                address + FORMAT.size > section.VirtualAddress + section.Misc_VirtualSize):
            raise ValueError('Build contract is not bounded read-only data: ' + str(path))
        result = decode_contract(pe.get_data(address, FORMAT.size))
    if result['component'] != expected_component:
        raise ValueError('Wrong compiled component identity: ' + str(path))
    return result


def validate_products(root, products):
    if set(products) != set(COMPONENTS):
        raise ValueError('A package needs game, server and host products')
    expected = source_contract(root)
    common = None
    for component, path in products.items():
        actual = read_contract(path, component)
        for key, value in expected.items():
            if actual[key] != value:
                raise ValueError('Stale/mixed build (' + component + '): ' + key + '; rebuild all products')
        comparable = {k: v for k, v in actual.items() if k != 'component'}
        if common is not None and comparable != common:
            raise ValueError('Products disagree on their compiled layout')
        common = comparable
    return common


def development_version(contract):
    version = '.'.join(str(contract['version_' + part]) for part in ['major', 'minor', 'patch'])
    return version + '-dev.' + contract['source_fingerprint'][:12]
