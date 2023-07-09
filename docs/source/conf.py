# -- Project information -----------------------------------------------------

project = 'OneWireHub'
project_full = "The OneWireHub is a sleek Arduino compatible (and many more platforms) library to emulate OneWire-Periphery with support for various devices & sensors"
copyright = '2023, Ingmar Splitt'
author = 'Ingmar Splitt'
release = '3.0.0'
builder = "html"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinxawesome_theme",
    "sphinx_sitemap",
    "myst_parser",
]

templates_path = ['_templates']
exclude_patterns = ["build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------

html_title = project
html_collapsible_definitions = True
html_copy_source = False

html_permalinks_icon = "<span>#</span>"
html_theme = "sphinxawesome_theme"
html_theme_options = {
    "show_scrolltop": True,
    "show_prev_next": True,
    "extra_header_links": {
        "Source": "https://github.com/orgua/OneWireHub",
    },
}
# TODO: https://sphinxawesome.xyz/how-to/options/
html_baseurl = "https://orgua.github.io/OneWireHub/"
html_extra_path = ["robots.txt"]
html_static_path = ["_static"]

sitemap_url_scheme = "{link}"
