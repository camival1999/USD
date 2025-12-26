"""
USD Package Import Tests

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

Basic smoke tests to verify package structure.
"""

import pytest


class TestPackageImports:
    """Test that all package modules can be imported."""

    def test_import_usd(self):
        """Test main package import."""
        import usd
        assert usd.__version__ == "0.1.0"
        assert usd.__author__ == "Camilo Valencia"

    def test_import_usd_core(self):
        """Test core module import."""
        from usd import core
        assert core is not None

    def test_import_usd_comm(self):
        """Test comm module import."""
        from usd import comm
        assert comm is not None

    def test_import_usd_devices(self):
        """Test devices module import."""
        from usd import devices
        assert devices is not None

    def test_import_usd_utils(self):
        """Test utils module import."""
        from usd import utils
        assert utils is not None


class TestGUIPackage:
    """Test GUI package."""

    def test_import_gui(self):
        """Test GUI module import."""
        import gui
        assert gui.__version__ == "0.1.0"

    def test_gui_main_exists(self):
        """Test that GUI entry point exists."""
        from gui import main
        assert callable(main)
