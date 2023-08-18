## 3.0.3

Changes:

- Add compat with ReScript React 0.11.x (4591a8a)

## 3.0.2

Fixes:

- Update React & React DOM peer dependencies (5abe9ac)

## 3.0.1

Fixes:

- Fixed name in bsconfig (c92e2aa)

## 3.0.0

Changes:

- Move to ReScript (f91817a)
- Move from reason-react to @rescript/react (f91817a)
- Rename project to `rescript-react-compat` (f91817a)

## 2.0.0

Changes:

- Make effect management preact-safe (fe4db7b)

## 1.0.1

Fixes:

- Pass an up to date self to `willUnmount` (0f49043)

## 1.0.0

Careful, this contains breaking changes!

Features:

- Add an `useMount` hook to easy migration (f39b6b1)

Changes:

- Bring our own implementation of `component` in order to remove unnecessary elements ([0f4a829](https://github.com/bloodyowl/reason-react-compat/commit/0f4a829ad70974e479fbc1262b28903d0d1a0ac6), motivations in the description)

Fixes:

- Pass an up-to-date self to `handle` handlers (b68c8bd)

## 0.4.0

Changes:

- Use useEffect instead of useLayoutEffect

## 0.3.0

Fixes:

- Remove buggy condition that'd prevent re-render

## 0.2.0

Changes:

- Remove namespace

## 0.1.1

Fixes:

- Fixed some typos in the config

## 0.1.0

Initial release
