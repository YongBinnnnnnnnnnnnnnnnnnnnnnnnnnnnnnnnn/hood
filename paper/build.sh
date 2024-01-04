#!/bin/bash
if ! test -f ~/Downloads/plantuml.jar; then
  curl -L -C - "https://github.com/plantuml/plantuml/releases/download/v1.2023.13/plantuml-1.2023.13.jar" -o ~/Downloads/plantuml.jar
fi

if ! apt list --installed graphviz | grep -q installed; then
  sudo apt install graphviz -y
fi

pumls=($(find graphics/puml/ -name "*.puml"|sed "s/\.puml$//"))

for puml in "${pumls[@]}"
do
  doit=0
  if ! test -f ${puml}.png; then
    doit=1
  elif [[ ${puml}.puml -nt ${puml}.png ]]; then
    doit=1
  fi
  if [ $doit -eq 1 ]; then
    echo "${puml}.puml"
    java -jar ~/Downloads/plantuml.jar -tpng "${puml}.puml"
  fi
done

read -p "Continue?" go_ahead

if [ $go_ahead != "y" ]; then
  exit
fi

latexmk -synctex=1 -file-line-error -pdf  paper.tex
#-interaction=nonstopmode 