# Documentation 

## Dependencies

The QB documentation system is built on the following technologies:

| Technology | Purpose                                                                                                                                                                                          |
|------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Sphinx     | Use Python's Docutils to convert reStructuredText to HTML.                                                                                                                                       |
| Doxygen    | Extracting API documentation in C++. Converts C++ documentation to XML.                                                                                                                          |
| Breathe    | Sphinx extension that can convert Doxygen’s XML output into individual reStructuredText directives (e.g. enables directives to show the documentation for a given function, class, macro, etc.). |
| Exhale     | Sphinx extension uses Doxygen’s XML output to create an entire set of reStructuredText pages to document the whole C++ library API. Uses Breathe underneath the hood.                            |

These dependencies can be installed by:

```
sudo apt install doxygen graphviz
python3 -m pip install sphinx==4.5.0 sphinx_rtd_theme==1.2.0 exhale==0.3.6 myst-parser==0.18.1
```

**NOTE**

If the `doxygen` version from the `apt` repository is outdated, please build and install it manually from the source:

```
sudo apt-get install flex bison
git clone https://github.com/doxygen/doxygen.git
cd doxygen/
mkdir build
cd build
cmake -G "Unix Makefiles" ..
sudo make install
```

To enable documentation build, pass `-DBUILD_DOCS=ON` to `cmake` when configuring the build.

## Directory structures

We structure this `docs/` directory as follows: 

```
docs/
├── index.rst
├── static
├── md
└── rst

```

- `index.rst`: top-level layout (control the overall document structure). It contains links to sub-pages (i.e., other `.rst` files) or Sphinx directives.

- `md`: directory contains all Markdown documents.

**Note**: the native markup format for Sphinx is reStructuredText, but we allow for Markdown inputs and just add a directive in the reStructuredText file to include those markdown documents using Sphinx's MyST extension, e.g.,

```
.. include:: md/some_markdown.md
   :parser: myst_parser.sphinx_
```   

- `rst`: directory contains all reStructuredText documents, e.g., those that `index.rst` refers to. 

These reStructuredText files might use Sphinx directives to include Markdown documents or place the Doxygen (C++)/autodoc (Python) auto-generated API documentations (e.g., for a particular class or file, etc.)


- `static`: where we put things like css stylesheets, logo icons, images/pictures, custom javascript files, etc.


**IMPORTANT**

If static images/figures/files to be embedded in markdown files, they need to be placed in the `static` directory rather than within the `md` directory. The reason is that `docutils`' `include` directive will basically embed the markdown content into the reStructuredText document, hence any links to external resources need to be consistent between markdown and reStructuredText documents.

## Output

Build artifacts are saved to the project build folder, i.e., `${CMAKE_BINARY_DIR}/docs`. 

In particular, `${CMAKE_BINARY_DIR}/docs/_build/html` is the HTML documentation site (self-contained), which will be installed to `${CMAKE_INSTALL_PREFIX}/docs/html`.