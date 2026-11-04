"""CPU and memory limits for the PC-side asset preparation workers."""
import ctypes
import os


def available_memory():
    try:
        if os.name == 'nt':
            class MemoryStatus(ctypes.Structure):
                _fields_ = [('length', ctypes.c_ulong), ('load', ctypes.c_ulong),
                            *[(name, ctypes.c_ulonglong) for name in
                              ('total', 'available', 'page_total', 'page_available',
                               'virtual_total', 'virtual_available', 'extended')]]
            status = MemoryStatus()
            status.length = ctypes.sizeof(status)
            if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
                return status.available
        else:
            return os.sysconf('SC_AVPHYS_PAGES') * os.sysconf('SC_PAGE_SIZE')
    except (AttributeError, OSError, ValueError):
        pass
    return None


def worker_count(kind, jobs=None):
    logical = os.cpu_count() or 1
    defaults = {'graphics': min(16, max(1, logical - 2)),
                'audio': min(8, max(1, logical // 2)),
                'music': min(4, max(1, logical // 2)),
                'ending': min(8, max(1, logical // 2))}
    requested = jobs if jobs is not None else os.environ.get('PVZ_PREP_' + kind.upper() + '_WORKERS')
    count = defaults[kind] if requested is None else int(requested)
    if not 1 <= count <= 32:
        raise ValueError('Preparation workers must be between 1 and 32')
    memory = available_memory()
    if memory is not None:
        # Leave room for Windows, the setup parent and retained source frames.
        per_worker = 384 if kind == 'ending' else 64
        count = min(count, max(1, (memory // 1048576 - 768) // per_worker))
    elif kind == 'ending':
        count = min(count, 2)
    return count
