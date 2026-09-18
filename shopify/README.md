# MURMUR Shopify

Recovery and source-control record for the MURMUR Shopify storefront.

## Current state — 18 Sep 2026

- **Repository:** `thotsl4yer69/murmur`
- **Primary domain:** `murmur.qd.je`
- **MAIN theme:** `MURMUR / SYSTEM 001 — FINAL 17 SEP`
- **Unpublished patched duplicate:** `MURMUR / SYSTEM 001 — PATCH 18 SEP`
- **Current paid offer:** `SYSTEM 001 — Founder's Reservation` — A$250
- **Development catalogue:** COWL, MOLT CORE, VEIL, CREST, PLUME, HUSH and MARK remain development records unless explicitly released

Shopify Admin is authoritative for the current customer-facing theme, catalogue, publication state and checkout availability. GitHub is the recovery/source-control copy and must be reconciled after direct Shopify changes.

## Public positioning

**GARMENTS AS SYSTEMS.**

Current public hierarchy:

- **MURMUR** — brand/platform
- **SYSTEM 001** — first public programme
- **COWL** — garment/shell architecture
- **MOLT CORE** — removable load-bearing/service carrier
- **Hardware** — separately validated control, sensing and observer modules

The immediate launch is SYSTEM 001, not the full historical product universe.

## Release controls

The live product template fails closed.

Purchase controls render only when:

1. the product has `murmur.release_approved = true`;
2. the selected variant has `murmur.release_approved = true`;
3. the selected variant has a positive price; and
4. the selected variant is available.

The Founder Reservation is intentionally released under this rule. Development garments remain gated.

Unsupported capability claims must stay out of customer-facing copy. Generated imagery must be identified as development imagery rather than physical prototype evidence.

## Brand/UI system

Palette:
- Bone `#F6F4EE`
- Ink `#101113`
- Indigo `#24263B`
- Mineral `#8F909A`
- Paper `#ECE8DF`

UI:
- square corners;
- thin rules;
- restrained motion;
- high-contrast primary actions;
- explicit keyboard focus states;
- preserve `prefers-reduced-motion`.

### 18 Sep contrast correction

The product-page black link-button inherited `.md a { color: inherit }`, which could override its intended light foreground. The patched theme raises selector specificity and explicitly preserves light text on the black primary action while keeping the transparent secondary action dark.

## Evidence/source hierarchy

1. Physical measured evidence.
2. Current release-gate package.
3. Shopify Admin/current MAIN theme.
4. Controlled build specification.
5. GitHub source/recovery copy.
6. Investor/marketing outputs.
7. Superseded manuals, renders and historical brand books.

Do not treat a pre-filled acceptance certificate or planned test as executed evidence.

## Recovery

1. Start from the current MAIN theme or latest approved duplicate.
2. Reconcile the current Shopify catalogue, pages, release metafields and collections.
3. Confirm SYSTEM 001 reservation purchase state separately from development products.
4. Confirm all CDN assets resolve.
5. QA desktop/mobile, contrast, keyboard focus, cart and policy links.
6. Run an end-to-end reservation checkout test before directing acquisition traffic.
7. Never infer release readiness from build success alone.

Do not commit Shopify access tokens, customer/order data, private keys, webhook secrets or payment credentials.
