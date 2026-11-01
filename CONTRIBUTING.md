# Contributing

Clear explanations and reproducible corrections are welcome. State the physics
question, settings, versions, seed and commands when reporting an issue. Include
relevant diagnostics; do not commit large event files, downloaded data or binaries.

Before proposing changes, run `bash scripts/test-analysis.sh`, then a short
`NEVENTS=100 bash scripts/check.sh`. For physics changes, explain the selection and
normalization and provide an appropriate validation beyond compilation.

Please distinguish a teaching approximation from a validated experimental analysis.
