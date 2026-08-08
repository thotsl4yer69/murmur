# MURMUR Store v4 — Release Candidate

Production-hardened storefront and operator backend for the MURMUR autonomy platform: **WEAR / MOLT**, **DEVICES**, **SYSTEMS**, and **The Murmuration**.

The project uses Node.js 22.13+, vanilla browser JavaScript, custom CSS, and Node's built-in SQLite module. It deliberately fails closed around unverified seller identity, live payment credentials, shipping/tax policy, inventory allocations, and physical sizing.

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

## Release architecture

The storefront includes a 14-product catalogue, technical product routes, responsive editorial commerce, Stripe Checkout integration, signed webhook verification, atomic inventory reservations, persistent order/fulfilment state, waitlist flows, SEO, security headers, Docker deployment, owner operations tooling and scale-migration documentation.

See `docs/` for deployment, commerce security, launch checklist, owner operations, MOLT interface, Murmuration protocol, validation and scale architecture.
