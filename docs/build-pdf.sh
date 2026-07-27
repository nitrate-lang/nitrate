#!/usr/bin/env bash
# Nitrate Documentation PDF Builder
# Produces a single aesthetic PDF from all markdown documentation files.
#
# Prerequisites:
#   - pandoc (https://pandoc.org)
#   - weasyprint OR pdflatex (for PDF generation)
#   - a Markdown-to-PDF engine
#
# This script concatenates all documentation files in order and
# renders them to a single PDF with a table of contents, page numbers,
# syntax-highlighted code blocks, and modern typography.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/_build"
OUTPUT_PDF="${OUTPUT_DIR}/nitrate-compiler-documentation.pdf"
METADATA_FILE="${OUTPUT_DIR}/metadata.yaml"
COMBINED_MD="${OUTPUT_DIR}/combined.md"

# Build output directory
mkdir -p "${OUTPUT_DIR}"

# Create metadata YAML for pandoc
cat > "${METADATA_FILE}" << 'YAML'
---
title: "Nitrate Compiler Documentation"
subtitle: "Comprehensive Architecture and Implementation Guide"
author: "The Nitrate Team"
date: "July 2026"
toc: true
toc-depth: 3
numbersections: true
colorlinks: true
lang: en-US
documentclass: book
fontsize: 11pt
geometry:
  - margin=1in
  - top=1.2in
  - bottom=1.2in
linestretch: 1.15
mainfont: DejaVu Serif
sansfont: DejaVu Sans
monofont: DejaVu Sans Mono
monofontoptions:
  - Scale=0.8
header-includes:
  - |
    ```{=latex}
    \usepackage{fancyhdr}
    \pagestyle{fancy}
    \fancyhead[LE,RO]{\slshape \leftmark}
    \fancyhead[RE,LO]{Nitrate Compiler Documentation}
    \fancyfoot[C]{\thepage}
    \usepackage{xltxtra}
    ```
  - |
    ```{=html}
    <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; line-height: 1.6; color: #333; max-width: 900px; margin: 0 auto; padding: 2em; }
    h1, h2, h3 { color: #1a1a1a; border-bottom: 1px solid #eaecef; padding-bottom: 0.3em; }
    h1 { font-size: 2em; }
    h2 { font-size: 1.5em; }
    code { background: #f6f8fa; border-radius: 3px; padding: 0.2em 0.4em; font-size: 85%; }
    pre code { background: #f6f8fa; border-radius: 6px; padding: 16px; overflow: auto; line-height: 1.45; display: block; }
    table { border-collapse: collapse; width: 100%; margin: 1em 0; }
    th, td { border: 1px solid #dfe2e5; padding: 6px 13px; text-align: left; }
    th { background-color: #f6f8fa; font-weight: 600; }
    blockquote { color: #6a737d; border-left: 4px solid #dfe2e5; padding: 0 1em; margin: 1em 0; }
    img { max-width: 100%; }
    nav.toc { background: #f8f9fa; border-radius: 8px; padding: 1.5em; margin: 2em 0; }
    nav.toc ul { list-style: none; padding-left: 1.5em; }
    nav.toc > ul { padding-left: 0; }
    nav.toc a { color: #0366d6; text-decoration: none; }
    nav.toc a:hover { text-decoration: underline; }
    .page-break { page-break-before: always; }
    </style>
    ```
---
YAML

# Collect all markdown files in reading order
FILES=(
    "${SCRIPT_DIR}/OVERVIEW.md"
    "${SCRIPT_DIR}/TRANSLATION.md"
    "${SCRIPT_DIR}/HIR.md"
    "${SCRIPT_DIR}/TYPE_SYSTEM.md"
    "${SCRIPT_DIR}/DIAGNOSTICS.md"
    "${SCRIPT_DIR}/DRIVER.md"
    "${SCRIPT_DIR}/LEXER.md"
    "${SCRIPT_DIR}/PARSER.md"
    "${SCRIPT_DIR}/RESOLVER.md"
    "${SCRIPT_DIR}/HINDLEY_MILNER.md"
    "${SCRIPT_DIR}/SOLVER.md"
    "${SCRIPT_DIR}/GENERICS_ARCHITECTURE.md"
    "${SCRIPT_DIR}/VALIDATION.md"
    "${SCRIPT_DIR}/EVALUATION.md"
    "${SCRIPT_DIR}/MANGLE.md"
    "${SCRIPT_DIR}/LLVM_CODEGEN.md"
    "${SCRIPT_DIR}/OPTIMIZATION.md"
    "${SCRIPT_DIR}/NSTRING.md"
    "${SCRIPT_DIR}/REFERENCE_SEMANTICS.md"
    "${SCRIPT_DIR}/LSP.md"
    "${SCRIPT_DIR}/PACKAGE_MANAGER.md"
    "${SCRIPT_DIR}/BUILD_SYSTEM.md"
)

echo "=== Nitrate Documentation PDF Builder ==="
echo ""
echo "Files to include:"
for f in "${FILES[@]}"; do
    echo "  - $(basename "$f")"
done
echo ""

# Verify all files exist
for f in "${FILES[@]}"; do
    if [ ! -f "$f" ]; then
        echo "ERROR: Missing file: $f"
        exit 1
    fi
done
echo "All files found."

# Add page breaks between documents
echo "" > "${COMBINED_MD}"
for f in "${FILES[@]}"; do
    echo "" >> "${COMBINED_MD}"
    echo "" >> "${COMBINED_MD}"
    cat "$f" >> "${COMBINED_MD}"
    echo "" >> "${COMBINED_MD}"
    echo '\pagebreak' >> "${COMBINED_MD}"
    echo "" >> "${COMBINED_MD}"
done

echo "Building PDF..."

# Try pandoc with weasyprint first, then pdf-engine, then latex
if command -v pandoc &> /dev/null; then
    # Try weasyprint for HTML→PDF
    if command -v weasyprint &> /dev/null; then
        echo "Using pandoc + weasyprint..."
        pandoc "${COMBINED_MD}" \
            --metadata-file="${METADATA_FILE}" \
            --from markdown \
            --to html5 \
            --pdf-engine=weasyprint \
            --output "${OUTPUT_PDF}" \
            --highlight-style=tango \
            --table-of-contents \
            --toc-depth=3 \
            --number-sections \
            --standalone \
            2>&1 && echo "PDF created successfully!" || echo "PDF build failed, trying alternate engine..."
    fi
    
    # Try xelatex if weasyprint failed or isn't available
    if [ ! -f "${OUTPUT_PDF}" ]; then
        if command -v xelatex &> /dev/null; then
            echo "Using pandoc + xelatex..."
            pandoc "${COMBINED_MD}" \
                --metadata-file="${METADATA_FILE}" \
                --from markdown \
                --to pdf \
                --pdf-engine=xelatex \
                --output "${OUTPUT_PDF}" \
                --highlight-style=tango \
                --table-of-contents \
                --toc-depth=3 \
                --number-sections \
                --standalone \
                2>&1 && echo "PDF created successfully!" || echo "PDF build failed."
        fi
    fi
    
    # Try pdflatex as fallback
    if [ ! -f "${OUTPUT_PDF}" ]; then
        if command -v pdflatex &> /dev/null; then
            echo "Using pandoc + pdflatex..."
            pandoc "${COMBINED_MD}" \
                --metadata-file="${METADATA_FILE}" \
                --from markdown \
                --to pdf \
                --pdf-engine=pdflatex \
                --output "${OUTPUT_PDF}" \
                --highlight-style=tango \
                --table-of-contents \
                --toc-depth=3 \
                --number-sections \
                --standalone \
                2>&1 && echo "PDF created successfully!" || echo "PDF build failed."
        fi
    fi

    # Try wkhtmltopdf as final fallback
    if [ ! -f "${OUTPUT_PDF}" ]; then
        if command -v wkhtmltopdf &> /dev/null; then
            echo "Using pandoc + wkhtmltopdf..."
            pandoc "${COMBINED_MD}" \
                --metadata-file="${METADATA_FILE}" \
                --from markdown \
                --to html5 \
                --pdf-engine=wkhtmltopdf \
                --output "${OUTPUT_PDF}" \
                --highlight-style=tango \
                --table-of-contents \
                --toc-depth=3 \
                --number-sections \
                --standalone \
                2>&1 && echo "PDF created successfully!" || echo "PDF build failed."
        fi
    fi
else
    echo "WARNING: pandoc not found. Install pandoc to generate PDF."
    echo "Installation:"
    echo "  Ubuntu/Debian: sudo apt install pandoc texlive-xetex weasyprint"
    echo "  macOS: brew install pandoc weasyprint"
    echo "  Or: pip install weasyprint pandoc"
fi

# Fallback: generate a self-contained HTML document if PDF fails
if [ ! -f "${OUTPUT_PDF}" ]; then
    echo ""
    echo "PDF generation was not possible. Generating HTML documentation..."
    OUTPUT_HTML="${OUTPUT_DIR}/nitrate-compiler-documentation.html"
    
    if command -v pandoc &> /dev/null; then
        pandoc "${COMBINED_MD}" \
            --metadata-file="${METADATA_FILE}" \
            --from markdown \
            --to html5 \
            --output "${OUTPUT_HTML}" \
            --highlight-style=tango \
            --table-of-contents \
            --toc-depth=3 \
            --number-sections \
            --standalone \
            --embed-resources \
            2>&1 && echo "HTML documentation created: ${OUTPUT_HTML}"
    else
        echo "pandoc not available. Cannot generate any output format."
        exit 1
    fi
fi

echo ""
echo "=== Build Complete ==="
echo "Output: ${OUTPUT_PDF}"
echo ""

# Show file size
if [ -f "${OUTPUT_PDF}" ]; then
    SIZE=$(du -h "${OUTPUT_PDF}" | cut -f1)
    PAGES=$(pdfinfo "${OUTPUT_PDF}" 2>/dev/null | grep Pages | awk '{print $2}' || echo "?")
    echo "Size: ${SIZE}"
    echo "Pages: ${PAGES}"
fi