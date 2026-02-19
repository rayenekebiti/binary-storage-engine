## Overview

* A low-level binary storage engine implementing:

* Fixed-size record layout

* Persistent file header (magic, version, next_id)

* Monotonic ID allocation

* Logical deletion (tombstones)

* File compaction

* In-memory hash index (id → offset)

## Design Goals

* Understand file I/O at byte level

* Separate metadata from data

* Maintain persistence invariants

* Implement restart-safe identity allocation
