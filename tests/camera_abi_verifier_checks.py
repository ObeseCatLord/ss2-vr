"""Synthetic negative controls for the compiled camera-wrapper verifier."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from abi_camera_forwarding import verify_camera_forwarding


def wrapper(frame=0x38):
    result = [f'sub $0x{frame:x},%esp', 'mov %ecx,%eax',
              f'mov 0x{frame:x}(%esp),%edx', 'movl $0x0,0x30(%esp)', 'mov %edx,0x34(%esp)']
    for word in range(12):
        destination = '(%esp)' if word == 0 else f'0x{word * 4:x}(%esp)'
        result += [f'mov 0x{frame + 4 + word * 4:x}(%esp),%edx', f'mov %edx,{destination}']
    return result + ['call 0x1234 <renderTrackedWeapon>', f'add $0x{frame:x},%esp', 'ret $0x30']


class CameraVerifier(unittest.TestCase):
    def test_current_and_larger_frame(self):
        for size in (0x38, 0x48):
            self.assertEqual(verify_camera_forwarding(wrapper(size))['camera_input_offset'], size + 4)

    def test_return_address_is_not_camera(self):
        code = wrapper()
        code[5] = 'mov 0x38(%esp),%edx'
        with self.assertRaises(ValueError): verify_camera_forwarding(code)

    def test_wrong_destination(self):
        code = wrapper(); code[6] = 'mov %edx,0x4(%esp)'
        with self.assertRaises(ValueError): verify_camera_forwarding(code)

    def test_arithmetic_before_store(self):
        code = wrapper(); code.insert(6, 'add $0x1,%edx')
        with self.assertRaises(ValueError): verify_camera_forwarding(code)

    def test_wrong_return_or_receiver(self):
        for index, replacement in [(-1, 'ret $0x34'), (1, 'mov %edx,%eax')]:
            code = wrapper(); code[index] = replacement
            with self.assertRaises(ValueError): verify_camera_forwarding(code)

    def test_extra_call(self):
        code = wrapper(); code.insert(2, 'call 0x5678 <unknown>')
        with self.assertRaises(ValueError): verify_camera_forwarding(code)

    def test_late_argument_overwrite(self):
        code = wrapper(); code.insert(-3, 'movl $0x0,(%esp)')
        with self.assertRaises(ValueError): verify_camera_forwarding(code)

    def test_wrong_discriminator(self):
        with self.assertRaises(ValueError): verify_camera_forwarding(wrapper(), sniper=True)
        verify_camera_forwarding(wrapper(), sniper=False)


if __name__ == '__main__': unittest.main()
