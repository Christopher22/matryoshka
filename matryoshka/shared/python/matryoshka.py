import ctypes
import os
from typing import Optional
from pathlib import Path


class Matryoshka:
    """
    A wrapper around the shared library.
    """

    def __init__(self, path: str):
        self.library_path = path
        self.library = None

    def __enter__(self):
        try:
            self.library = ctypes.WinDLL(self.library_path)
        except OSError:
            self.library = None
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        del self.library

    def __bool__(self):
        return self.library is not None

    @classmethod
    def find(cls, name: str = "Matryoshka.dll") -> Optional[str]:
        """
        Find a shared library in PATH.
        :param name: Name of the shared library
        :return: Path to the library or None if it is not found
        """
        for path in os.environ["PATH"].split(os.pathsep):
            file = Path(path) / name
            if file.is_file():
                return file
        return None
