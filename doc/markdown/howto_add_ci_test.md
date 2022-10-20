# How to add a CI test to GitLab

## Structure of GitLab CI
GitLab CI (Continuous Integration) is configured by a file named [`.gitlab-ci.yml`](.gitlab-ci.yml) that is at the root of the repository.
For qbOS, the aim is for CI tests (referred from here on as: **jobs**) to use one of the formats listed below:

* C++ unit tests: **Google Test** [gtest] format, or
* Python, or other e.g. integration tests, regression tests: **PyTest** format

#### Why do we use these formats?  
Primarily to allow a standard report format (JUnit) to be easily created and sent for rendering and summarisation by the GitLab UI.

#### What does the high-level configuration of the CI look like?

In `.gitlab-ci.yml`:
```yaml
image:
        name: xacc/xacc
        entrypoint: [""]
stages:
        - build
        - test
        - deploy
variables:
        GIT_SUBMODULE_STRATEGY: normal
before_script:
        - echo $PWD
        - ls -l
        - apt update

build-1:
        stage: build
        script:
                - echo $PWD

test-1:
        stage: test
        script:
                - echo $PWD

test-2:
        stage: test
        script:
                - echo $PWD

deploy-1:
        stage: deploy
        script:
                - echo $PWD                
                
```

#### What does `image` mean?
This is the Docker container image to use as the base system for all tests.  We use the ORNL release of XACC in DockerHub: `xacc/xacc`

#### What does `stages` mean?
A CI pipeline is divided into so called **stages**.  We have a 3-stage pipeline and the stages are declared here using the names:
* `build` - this contains tasks that compile shared libraries and dependencies
* `test` - this contains the driver code for jobs
* `deploy` - this contains tasks that produce the tarball needed for a subsequent new release of qbOS.

Later in this document, where ever you see: `<stage-name>`, replace it with one of the values from the above.

#### What does `variables` do?
Here, we set `GIT_SUBMODULE_STRATEGY` to `normal`.  The effect of this is to set up submodules that are used in the project prior to compiling and testing.

#### What does `before_script` mean?
Commands that need to run **prior** to all jobs are placed here.  In our case, it's simply: `apt update`.

## Example of a job

In this particular `.gitlab-ci.yml`, the first job is named `build-1`:
```yaml
build-1:
        stage: build
        script:
                - echo $PWD
```
The template for a job is:
```yaml
<job-name>:
        stage: <stage-name>
        script:
                - /bin/bash <path from repository root to BASH job-driver-script 1>
                - /bin/bash <path from repository root to BASH job-driver-script 2>
                .
                .
                .
``` 
## Example of PyTest job
Let's go through an exercise in adding a PyTest job.
```yaml
my_new_module_job_1:
    stage: test
    script: tpls/my_new_module/test/test-CI-210629.sh`

```
Here is the Bash **job driver script**: `tpls/my_new_module/test/test-CI-210629.sh` 


> **`Important`:** Call all test drivers from the **repository's root directory**. 

> **`test/`** should be provided by all modules as a subdirectory where test driver code and test cases are kept.

```bash    
    ORIGINDIR=$(pwd)
    source "${ORIGINDIR}"/ci/include/function_prereqs.sh
    source "${ORIGINDIR}"/ci/include/function_setpaths.sh
    function_CI-210629
    exit 0
```
* [`function_prereqs.sh`](ci/include/function_prereqs.sh) - This is a helper function that installs common prerequisite packages
* [`function_setpath.sh`](ci/include/function_setpaths.sh) - This is a helper function that sets up environment variables
* `function_CI-210629` - This is where you call PyTest and set exit codes if the test fails.  For this example, it contains:
```bash
function_CI-210629() {
    ORIGINDIR=$(pwd)
    if [[ -z "${FN_PREREQS}" ]];
    then
      source "${ORIGINDIR}"/ci/include/function_prereqs.sh
    else       
      echo "- Prerequisites already installed...skipping function_prereqs()"
    fi  
    if [[ -z "${FN_SETPATHS}" ]];
    then
      source "${ORIGINDIR}"/ci/include/function_setpaths.sh
    else       
      echo "- Paths already set...skipping function_setpaths()"
    fi  
    echo " ***"
    echo " *** CI-210629: pytest : my new unit test"
    echo " ***"
    cd "${ORIGINDIR}"/tpls/my_new_module
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QE_INSTALL_PATH}"
    make
    make install
    pytest ../test/test_my_new_module.py --junitxml=report.xml || exit 210629
    echo " ***"
    echo " *** Passed *** - CI-210629: pytest : my new unit test"
    echo " ***"
}
```
> **Note:** the `--junitxml=report.xml` option produces the XML report that GitLab will use for rendering job summaries.

For this example `test_my_new_module.py` contains:

```python
import os
import pytest
def test_my_new_module_1() :
    print("* CI_210629:")
    print("* Imports qbemulator Python module and calls qb12(), which should give a default setup for 12 qubits.")
    import qbemulator as qb
    tqb = qb.qbqe()
    tqb.qb12()
    assert (tqb.qn[0][0]) == 12, "[my_new_module] failed test: CI_210629"
```

## Example of a GTest (C++) job

