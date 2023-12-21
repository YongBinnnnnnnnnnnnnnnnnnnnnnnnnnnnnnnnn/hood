#!/bin/sh
if ! test -f ~/Downloads/plantuml.jar; then
  curl -L -C - "https://github.com/plantuml/plantuml/releases/download/v1.2023.13/plantuml-1.2023.13.jar" -o ~/Downloads/plantuml.jar
fi

if ! apt list --installed graphviz | grep -q installed; then
  sudo apt install graphviz -y
fi

java -jar ~/Downloads/plantuml.jar -tpng graphics/puml/*.puml
latexmk -synctex=1 -file-line-error -pdf  paper.tex
#-interaction=nonstopmode 