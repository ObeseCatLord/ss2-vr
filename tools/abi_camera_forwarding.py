"""Bounded verifier for this project's scalar x86 camera-forwarding wrappers.

Unknown compiler shapes are rejected, never silently treated as equivalent.
"""
import re


def verify_camera_forwarding(instructions, sniper=None):
    code = [re.sub(r'\s+', ' ', line.strip()) for line in instructions]
    if not code:
        raise ValueError('Empty wrapper')
    frame = re.fullmatch(r'sub \$(0x[0-9a-f]+),%esp', code[0])
    if not frame:
        raise ValueError('Unknown wrapper stack reservation')
    size = int(frame.group(1), 16)
    if size < 0x38:
        raise ValueError('No space for camera, boolean and caller provenance')
    loads = [(i, int(m.group(1), 16)) for i, line in enumerate(code)
             if (m := re.fullmatch(r'mov 0x([0-9a-f]+)\(%esp\),%edx', line))]
    # Entry ESP points to the return address, then twelve camera words. The
    # return address is a separate provenance argument, NOT camera word zero.
    expected = [size] + list(range(size + 4, size + 0x34, 4))
    if [offset for _, offset in loads] != expected:
        raise ValueError('Input words do not match return address plus 48-byte camera')
    if 'mov %ecx,%eax' not in code[:loads[0][0]]:
        raise ValueError('Native receiver is not forwarded in the expected register')
    for word, (at, _) in enumerate(loads):
        offset = 0x34 if word == 0 else (word - 1) * 4
        destination = '(%esp)' if offset == 0 else f'0x{offset:x}(%esp)'
        store = f'mov %edx,{destination}'
        following = code[at + 1:]
        try:
            end = following.index(store)
        except ValueError as exc:
            raise ValueError('Missing unchanged-word output store') from exc
        if any(not re.fullmatch(r'movl \$0x[01],0x30\(%esp\)', line)
               for line in following[:end]):
            raise ValueError('Word changed or control flow intervened before forwarding')
    discriminators = [line for line in code if re.fullmatch(r'movl \$0x[01],0x30\(%esp\)', line)]
    if len(discriminators) != 1:
        raise ValueError('Missing sniper/non-sniper discriminator')
    if sniper is not None and discriminators[0] != f'movl $0x{int(sniper):x},0x30(%esp)':
        raise ValueError('Wrong sniper/non-sniper discriminator')
    calls = [line for line in code if line.startswith('call ')]
    if len(calls) != 1 or 'renderTrackedWeapon' not in calls[0]:
        raise ValueError('Unexpected wrapper calls')
    at = code.index(calls[0])
    if at <= loads[-1][0] or code[at + 1:at + 3] != [f'add $0x{size:x},%esp', 'ret $0x30']:
        raise ValueError('Stack restoration/callee cleanup differs')
    expected_code = [code[0], 'mov %ecx,%eax']
    for word, source in enumerate(expected):
        offset = 0x34 if word == 0 else (word - 1) * 4
        destination = '(%esp)' if offset == 0 else f'0x{offset:x}(%esp)'
        expected_code += [f'mov 0x{source:x}(%esp),%edx', f'mov %edx,{destination}']
    expected_code += [calls[0], f'add $0x{size:x},%esp', 'ret $0x30']
    body = [line for line in code[:at + 3] if line != discriminators[0]]
    if body != expected_code:
        raise ValueError('Unexpected instruction can modify forwarded arguments or receiver')
    return {'camera_bytes': 48, 'camera_input_offset': size + 4,
            'caller_provenance_forwarded_separately': True}
