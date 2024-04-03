# Security Policy

Any flaws in code located in a `*-testing` branch or a release are bugs and should be reported. 

- If the flaw yields an `INTERNAL COMPILER ERROR,` submit the issue via GitHub Issues.
- If the flaw yields a SEGMENTATION FAULT or other CRASH, submit the issue via GitHub Issues and mark it as `critical.`
- If the flaw yields an incorrect compilation (output is invalid) OR if, in your opinion, it compromises the confidentiality, integrity, or availability of the User's system, file a brief issue via GitHub Issues with minimal details of the effect. The maintainers will contact you to resolve the flaw.

Suppose the project is in a `pre-release` stage OR the code is in a `*-dev` branch. In that case, all code is assumed to be untested, so don't bother filing bug reports based on it alone.
