# Minimal Binary Storage Engine (C++)

This project is a learning-focused implementation of a minimal binary
storage engine written in C++.
updates of this project to make it much useful are soon coming.

It uses a fixed-size record layout stored directly on disk, with:
- a persistent file header
- append-only record insertion
- update-in-place modification
- logical deletion via flags
- compaction to remove inactive records
- an in-memory index rebuilt on startup

## File Format

[ header ][ record ][ record ][ record ] ...

- header: magic, version, next_id
- record: { id, fixed-size payload, flags }

## Supported Operations

- add_records(name)
- modify_records(id, new_name)
- set_inactive(id)
- clear_inactive_records()

## Design Constraints

- single-threaded
- fixed-size records
- no WAL / crash recovery
- no concurrency
- educational scope

## Motivation

This project was built to understand:
- binary file layouts
- offset-based indexing
- invariants in storage systems
- safe use of memcpy / fixed buffers

## Limitations / Next Steps

- crash consistency
- write-ahead logging
- persistent index
- variable-length records
