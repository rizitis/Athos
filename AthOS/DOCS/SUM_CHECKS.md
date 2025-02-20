## Accepted VARS are

- SHA512SUM=
- SHA256SUM=
- MDSCHK=
- or empty for not check sums...

| Scenario                  | Expected Outcome                        |
|---------------------------|----------------------------------------|
| ✅ SHA512 checksum passes | Skips SHA256/MD5 & continues          |
| ✅ SHA256 checksum passes | Skips MD5 & continues                 |
| ✅ MD5 checksum passes    | Continues script without exit         |
| ❌ All checksums fail     | Deletes & re-downloads file           |
| ❌ No checksum provided   | Skips check & continues               |
