export FN_SETPATHS=1
export ORIGINDIR=$(pwd)

export QE_DIR=/mnt/qb
export QE_INSTALL_PATH="${QE_DIR}"

mkdir -p "${QE_DIR}"
mkdir -p "${QE_INSTALL_PATH}/bin"

export EXATN_DIR="${QE_DIR}"/exatn-local
ln -s "${ORIGINDIR}"/tpls/exatn-base-precomp  "${EXATN_DIR}"

export XACC_DIR="${QE_DIR}"/xacc-local
ln -s "${ORIGINDIR}"/tpls/xacc-base-precomp "${XACC_DIR}"

export LD_LIBRARY_PATH="${QE_INSTALL_PATH}/lib":"${QE_INSTALL_PATH}/mpich-local/lib":"${XACC_DIR}/lib":"${XACC_DIR}/plugins":"${EXATN_DIR}/lib":"${EXATN_DIR}/plugins":"${LD_LIBRARY_PATH}"
export PATH="${QE_INSTALL_PATH}/mpich-local/bin":"${QE_INSTALL_PATH}/bin":"${PATH}"
export PYTHONPATH="${QE_INSTALL_PATH}/lib":"${XACC_DIR}":"${PYTHONPATH}"
