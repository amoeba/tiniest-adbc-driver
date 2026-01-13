import pytest
import os
import sys

from adbc_driver_manager import dbapi


def get_library_extension():
    """Get the platform-specific shared library extension."""
    if sys.platform == "darwin":
        return ".dylib"
    elif sys.platform == "win32":
        return ".dll"
    else:
        return ".so"


def get_library_name():
    """Get the platform-specific library name."""
    if sys.platform == "win32":
        return "tiny" + get_library_extension()
    else:
        return "libtiny" + get_library_extension()


def test_driver():
    driver_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "..",
        get_library_name()
    )
    assert os.path.exists(driver_path), f"Driver library not found at {driver_path}"

    with dbapi.connect(driver=driver_path) as conn:
        with conn.cursor() as cursor:
            pass
