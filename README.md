# MURMUR Store v4 — SYSTEM 001

Storefront and operator source for the MURMUR wearable platform, currently focused on **SYSTEM 001**: an architectural garment system built around a removable carrier, serviceable hardware and direct wearer control.

## Current commercial state — 18 Sep 2026

- **Primary storefront:** Shopify at `murmur.qd.je`
- **Live Shopify theme:** `MURMUR / SYSTEM 001 — FINAL 17 SEP`
- **Current paid offer:** `SYSTEM 001 — Founder's Reservation` at A$250
- **Development products:** COWL, MOLT CORE and the wider WEAR range remain non-release development records
- **Release rule:** Shopify purchase controls are exposed only where the product and selected variant both carry `murmur.release_approved = true`

The legacy 14-product/storefront architecture remains useful as research history, but it is not the current launch scope. The launch path is SYSTEM 001 first.

## Source-of-truth hierarchy

1. Measured physical evidence and recorded test results.
2. Current release-gate package for engineering readiness.
3. Shopify Admin + current MAIN theme for what customers can actually see and buy.
4. Current controlled build specification for assembly.
5. GitHub `main` for source code and recovery material, reconciled against Shopify after live theme changes.
6. Investor, marketing and campaign material as derivative outputs only.
7. Superseded manuals, brand books and renders as archive/reference material.

A later controlled release gate overrides earlier template or provisional acceptance records.

## Shopify

Shopify implementation notes and recovery state live under [`shopify/`](shopify/README.md).

A patched duplicate theme, `MURMUR / SYSTEM 001 — PATCH 18 SEP`, contains the 18 Sep button-contrast correction and is kept unpublished until visual QA/publishing.

## Commands

```bash
npm test
npm run audit
npm run preflight
npm run ops -- dashboard
npm run ops -- queue
npm run backup
npm start
```

## Release principle

**Build proof, not mythology.** A research capability, concept render or planned interface does not become a released product claim until the relevant physical evidence and approved production specification support it.
