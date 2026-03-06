#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASKELETON_BIN="${ROOT_DIR}/askeleton"
export ASKELETON_HOME="${ASKELETON_HOME:-$ROOT_DIR}"

OUT_BASE="${1:-/tmp/askeleton_real_demo_$(date +%Y%m%d_%H%M%S)}"
mkdir -p "$OUT_BASE"

pause_or_quit() {
  local ans
  read -r -p "Pulsa Enter para continuar (q para salir): " ans
  if [[ "$ans" == "q" || "$ans" == "Q" ]]; then
    echo "Demo interrumpida por el usuario."
    exit 0
  fi
}

print_sep() {
  echo
  echo "================================================================"
}

explain_reason() {
  local reason="$1"
  case "$reason" in
    abstract_record)
      echo "  - $reason: hay tipos abstractos que no se pueden instanciar automáticamente."
      ;;
    non_public_lifecycle)
      echo "  - $reason: destructor/ciclo de vida no público para el fixture generado."
      ;;
    unsupported_indirection)
      echo "  - $reason: punteros/referencias con niveles de indirección no soportados."
      ;;
    unsupported_type_shape)
      echo "  - $reason: forma de tipo compleja fuera del materializador actual."
      ;;
    unsupported_pointer_pointee)
      echo "  - $reason: el tipo apuntado por un puntero no se puede materializar."
      ;;
    incomplete_type)
      echo "  - $reason: el tipo es incompleto (declaración sin definición visible)."
      ;;
    coverage_policy_mutable_parameter)
      echo "  - $reason: en strict se omiten parámetros mutables por política."
      ;;
    coverage_policy_instance_construction)
      echo "  - $reason: en strict se evita construcción no-default de instancia."
      ;;
    *)
      echo "  - $reason: ver detalle en doc/SkipReasons.md"
      ;;
  esac
}

summarize_report() {
  local report_path="$1"
  python3 - "$report_path" << 'PY'
import json, sys
p = sys.argv[1]
with open(p, 'r', encoding='utf-8') as f:
    data = json.load(f)
summary = data.get('summary', {})
coverage = summary.get('coverage', {})
found = coverage.get('found', 0)
gen = coverage.get('generated', 0)
skip = coverage.get('skipped', 0)
rate = coverage.get('generation_rate', 0.0) * 100.0
print(f"Resumen técnico: found={found}, generated={gen}, skipped={skip}, generation_rate={rate:.2f}%")
if rate >= 95:
    print("Lectura recomendada: excelente caso para demo de éxito (alta cobertura de generación).")
elif rate >= 60:
    print("Lectura recomendada: caso real equilibrado (genera mucho, pero también enseña límites).")
else:
    print("Lectura recomendada: caso difícil; útil para explicar límites y razones de skip.")
reasons = summary.get('by_reason', {})
if reasons:
    print("Razones de skip detectadas:")
    for k, v in sorted(reasons.items(), key=lambda kv: (-kv[1], kv[0])):
        print(f"  {k}:{v}")
else:
    print("Razones de skip detectadas: ninguna.")
PY
}

show_reason_explanations() {
  local report_path="$1"
  local reasons
  reasons="$(python3 - "$report_path" << 'PY'
import json, sys
with open(sys.argv[1], 'r', encoding='utf-8') as f:
    data = json.load(f)
reasons = (data.get('summary', {}).get('by_reason', {}) or {}).keys()
print("\n".join(sorted(reasons)))
PY
)"

  if [[ -z "$reasons" ]]; then
    echo "Interpretación: no hay skips en este caso."
    return
  fi

  echo "Interpretación de razones de skip:" 
  while IFS= read -r r; do
    [[ -z "$r" ]] && continue
    explain_reason "$r"
  done <<< "$reasons"
}

run_case() {
  local case_id="$1"
  local title="$2"
  local build_path="$3"
  local source_file="$4"
  local case_note="$5"

  local out_dir="${OUT_BASE}/${case_id}_out"
  local report_path="${OUT_BASE}/${case_id}_report.json"

  print_sep
  echo "[$case_id] $title"
  echo "$case_note"
  echo "Build path: $build_path"
  echo "Source: $source_file"
  echo "Out: $out_dir"
  echo "Report: $report_path"

  echo
  echo "Subpaso 1/3: ejecutar ASkeleTon"
  pause_or_quit

  ASKELETON_HOME="$ASKELETON_HOME" "$ASKELETON_BIN" \
    -p "$build_path" \
    --report="$report_path" \
    --out-dir "$out_dir" \
    "$source_file"

  echo
  echo "Subpaso 2/3: explicación automática del resultado"
  pause_or_quit

  summarize_report "$report_path"
  show_reason_explanations "$report_path"

  echo
  echo "Subpaso 3/3: ahora abres tú manualmente"
  pause_or_quit

  echo "Carpetas/targets generados:" 
  find "$out_dir" -mindepth 1 -maxdepth 1 -type d | sort
  echo
  echo "Puedes abrir manualmente ahora:"
  echo "  - Report: $report_path"
  echo "  - Salida: $out_dir"
  echo "  - Si quieres ejemplos rápidos:"
  echo "    find '$out_dir' -maxdepth 2 -type f | sort | head -n 30"
  echo
  echo "Caso [$case_id] completado."
}

print_sep
echo "Demo de Proyectos Reales (ASkeleTon)"
echo "ASKELETON_HOME: $ASKELETON_HOME"
echo "Salida base: $OUT_BASE"

echo
cat << 'TXT'
Qué vas a ver:
1) OpenSSL (caso corto de éxito)
2) tinyxml2 (caso real equilibrado)
3) sqlite shell.c (caso difícil para enseñar límites)
TXT

run_case \
  "A" \
  "OpenSSL: ctype.c" \
  "$ROOT_DIR/examples/openssl-3.6.1" \
  "$ROOT_DIR/examples/openssl-3.6.1/crypto/ctype.c" \
  "Objetivo: mostrar un caso real con alta tasa de generación."

run_case \
  "B" \
  "tinyxml2: tinyxml2.cpp" \
  "$ROOT_DIR/examples/tinyxml2-master/build" \
  "$ROOT_DIR/examples/tinyxml2-master/tinyxml2.cpp" \
  "Objetivo: mostrar volumen real con mezcla de generados/skips explicables."

run_case \
  "C" \
  "sqlite: shell.c" \
  "$ROOT_DIR/examples/sqlite-src-3510200" \
  "$ROOT_DIR/examples/sqlite-src-3510200/shell.c" \
  "Objetivo: enseñar límites de forma transparente con reporte de razones."

print_sep
echo "Demo de proyectos reales completada."
echo "Todo quedó en: $OUT_BASE"
