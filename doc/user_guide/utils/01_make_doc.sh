#!/bin/bash
# (c) 2021 Quantum Brilliance Pty Ltd
#
# 01_make_doc.sh : driver script for building qbOS user guides
# 
# Usage:
#         utils/01_make_doc.sh
#

# Step 1 of 5: Install the prerequisite libraries

apt-get -y install build-essential
apt-get -y install libpython3-dev python3 python3-pip
apt-get -y install pkg-config
apt-get -y install libcurl4-openssl-dev
apt-get -y install libssl-dev
apt-get -y install unzip

python3 -m pip install cmake
python3 -m pip install mkdocs
python3 -m pip install lib3to6
python3 -m pip install markdown-katex


# Step 2 of 5: Unzip the Notion exported Markdown

NOTION_EXPORT_ZIP=$(find src/ -type f -name *.zip)
echo "Extracting Markdown from: ${NOTION_EXPORT_ZIP}"
rm -rf build/markdown/*
rm -rf build/jupyterlab/*
unzip "${NOTION_EXPORT_ZIP}" -d build/markdown

INDEX_MD=$(find build/markdown/ -maxdepth 1 -type f \( -name '*.md' ! -name 'index.md' \) )
echo "index.md will be created from: ${INDEX_MD}"

NHA=$(echo "${INDEX_MD}" | cut -d'.' -f1 | rev | cut -d' ' -f1 | rev)
TARGET=$(date +%y%m%d)
SRCDD=$(find build/markdown -maxdepth 1 -type d | grep $NHA)
TARGETINDEX_MD=$(echo "${INDEX_MD}" | sed 's/'"${NHA}"'/'"${TARGET}"'/g')
TARGETD=$(echo "${SRCDD}" | sed 's/'"${NHA}"'/'"${TARGET}"'/g')

sed -i -e 's/'"%20${NHA}"'/'"%20${TARGET}"'/g' "${INDEX_MD}"
find build/markdown/ -type f -name '*.md' -exec sed -i -e 's/'"%20${NHA}"'/'"%20${TARGET}"'/g' '{}' \;

mv "${INDEX_MD}" "${TARGETINDEX_MD}"
mv "${SRCDD}" "${TARGETD}"
echo
echo "DEBUG: TARGETINDEX_MD : ${TARGETINDEX_MD}"
echo

cp "${TARGETINDEX_MD}" build/markdown/index.md
cp -pr build/markdown/* build/jupyterlab/.
cp "${TARGETINDEX_MD}" build/jupyterlab/index.md
mv "${TARGETINDEX_MD}" build/markdown/00_full.md

# Step 3 of 5: Apply correction for Katex/Latex to comply with Python Markdown
find build/markdown/ -type f -name '*.md' -exec sed -i -e 's/\$\$\(.*\)\$\$/\n\`\`\`math\n\1\n\`\`\`/g' '{}' \;
find build/markdown/ -type f -name '*.md' -exec sed -i -e 's/\$\([^$]*\)\$/\$`\1`\$/g' '{}' \;

# Handle Notion output [[Display]()](URL) at [Display](URL)
sed -i -r 's/\[\[(.*)\]\(\)\]/[\1]/g' build/markdown/index.md
sed -i -r 's/\[\[(.*)\]\(\)\]/[\1]/g' build/markdown/00_full.md

# JupyterLab: Change \ket{0} to explicit "|0>" 
# sed -i -r 's/\\ket\{([0-9A-Za-z\\]*)\}/|\1\\rangle/g' build/jupyterlab/index.md
# JupyterLab: Handle Notion output [[Display]()](URL) at [Display](URL)
sed -i -r 's/\[\[(.*)\]\(\)\]/[\1]/g' build/jupyterlab/index.md
# Step 4 of 5: Generate HTML using `mkdocs`

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

cd build
if [[ ! -L "docs" ]];
then
    ln -s markdown docs
fi
echo "Generating HTML documentation..."
csplit  --suffix-format="%02d.md" docs/index.md /^#\ [0-9]/ "{*}"
mv xx*.md docs/.
mv docs/00_full.md docs/index.md
rm docs/xx00.md
cp -pr css docs/.
python3 -m mkdocs build
cd ..

