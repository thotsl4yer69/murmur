# MURMUR Shopify

Canonical source for the Shopify implementation of the MURMUR storefront.

## Source of truth

- **Repository:** `thotsl4yer69/murmur`
- **Shopify draft theme:** `MURMUR / SYSTEM 01`
- **Theme role:** unpublished
- **Published store theme:** Horizon remains untouched until the MURMUR draft is approved.

This directory exists so the Shopify theme is never the only copy of the work.

## Structure

- `theme/` — customized Horizon theme files used by the draft.
- `store-state-2026-09-09.json` — recovery snapshot of catalogue, collections, navigation and verified remote theme checksums.

## Brand system

**GARMENTS AS SYSTEMS.** MURMUR should read first as an experimental fashion house and modular design platform. Technology is structural, removable and serviceable rather than decorative.

Palette:
- Bone `#E8E3D9`
- Ink `#0F1115`
- Indigo `#24263B`
- Mineral `#8F909A`
- Paper `#F2EEE6`

Typography and UI:
- Anonymous Pro for heading/accent roles through Horizon settings.
- Inter for body copy.
- Major headings uppercase, large-scale and tight.
- Navigation/micro-labels uppercase with deliberate tracking.
- Square corners, thin 1px rules, no decorative pills.
- Restrained motion; preserve `prefers-reduced-motion`.

Information architecture:
- `CURRENT` — current editorial expression.
- `WEAR` — visible garment layers.
- `MOLT` — removable structural backbone.
- `SYSTEM` — platform explanation.
- `CONTACT` — contact surface.

Product naming uses short system names: `COWL`, `VEIL`, `CREST`, `PLUME`, `HUSH`, `MARK`, `MOLT CORE`.

## Prototype merchandising rule

Until release readiness:
- products remain DRAFT;
- `$0` pricing is not presented as customer-facing pricing;
- purchase and variant controls stay out of the draft product template;
- product pages function as design dossiers;
- unsupported capability claims stay out of fashion-facing copy.

## Recovery

1. Duplicate a compatible Horizon theme in Shopify.
2. Apply the files under `theme/` to the duplicate, preserving paths.
3. Reconcile catalogue, collections and navigation against `store-state-2026-09-09.json`.
4. Confirm all CDN image URLs resolve.
5. Preview every breakpoint before publishing.
6. Never overwrite the live theme as the first recovery step.

Shopify may normalize comments/formatting in JSON theme files. A checksum mismatch is a signal to inspect the diff, not proof of data loss.

Do not commit Shopify access tokens, customer/order data, private keys, webhook secrets or payment credentials.

## Release rule

Do not publish solely because the code builds. Publish only after responsive visual QA, final imagery, real sizing/pricing, claims matched to demonstrated prototypes, and checkout/policy content are complete.
