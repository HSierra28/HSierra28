# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

# conf.py

import os
import sys
sys.path.insert(0, os.path.abspath('../src'))

# Project info
project = 'RFID Access Control + HMI'
author = 'Hammurabi Sierra'
release = '1.0'

# Extensions
extensions = ['breathe']

# Breathe configuration
breathe_projects = { "RFID-HMI": "../generated-docs/xml" }
breathe_default_project = "RFID-HMI"

# Templates and static paths
templates_path = ['_templates']
exclude_patterns = []

# HTML output
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
