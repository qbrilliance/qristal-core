# qbOS

# 1.0 Background

Previous software projects at QB have organically derived a repository structure, usually following loosely an already well understood public software project's structure.  For example, in the Quantum Emulator, the XACC public project guided the repository structure.

Here we draw on the past lessons to propose a structure that fulfils these requirements :

1. Presents to a developer a **recognisable structure if working across different QB projects**
2. Facilitates **upstream** code contribution
3. Enhances the scope and test coverage of CI (Continuous Integration) testing
4. Unifies documentation for users and developers together with the source code
5. Simplifies public release through channels that are followed by industry observers

## 1.1 General concepts guiding the repository structure

[Old QE Pages](https://www.notion.so/Old-QE-Pages-e873a62d490b42f3bbdb0000fc76b887)

[Documentation (experimental)](https://www.notion.so/Documentation-experimental-b92dff4ccabc43dbaa572c950e9916ab)

# 2.0 Proposed repository structure

- qbOS
    - assets
        - html
        - markdown

            In a tree hierarchy that mirrors the hierarchy under /qbOS , place one summary document here in Markdown format  

    - cmake
    - src

        Any subdirectory under `src` can optionally have:
        `test/`
        in which a test harness driver and test code is kept.

        Any subdirectory under `src` can optionally have: `build/` in which compilation can be performed for the subdirectory and performed recursively for lower subdirectories contained thereunder.

        - compilers
        - pretranspilers
        - utils
            - parsers
            - string_manipulation
            - 
        - accelerators
            - comm_protocols
        - 
    - gui
        - v1commandlines
            - qbtheia
        - v2jupyter
        - qbquirk
    - tpls

        This is where third-party libraries (tpls) are placed.

    - plugins
        - base_algorithms

            Place your algorithms that are reusable by other modules in QB here

        - noise_models
        - q_finance_module

        - q_random_circuit
        - q_decoder_module

    - mathtools
        - linearalgebra

        - statistics
    - circuitexamples
        - DJ
        - BV
        - random_circuit_sampling
    - lowermodules
        - qbbios
        - qbqudi

[Preliminary Structures](https://www.notion.so/Preliminary-Structures-089e4057b1a34b4c8a7581027cb819e2)

[Modules](https://www.notion.so/Modules-5992600cac064873ba049c9664472f87)