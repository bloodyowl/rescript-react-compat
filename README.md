# rescript-react-compat

> An alternative upgrade path for ReasonReact

## Installation

```console
$ yarn add rescript-react-compat
```

or

```console
$ npm install --save rescript-react-compat
```

Then add `rescript-react-compat` to your `bsconfig.json` `bs-dependencies` field.

## ReactCompat.useRecordApi

Enables you to wrap your existing `ReasonReact.statelessComponent` and `ReasonReact.reducerComponent` through a React hook.

```reason
[@react.component]
let make = () => {
  ReactCompat.useRecordApi({
    ...ReactCompat.component,
    render: _ =>
      <div> "Helloworld!"->ReasonReact.string </div>
  })
}
```

### Upgrade path

#### Stateless components

For implementation files (`.re`)

```diff
-let component = ReasonReact.statelessComponent("MyComponent");

+[@react.component]
- let make = _ => {
+ let make = () => {
+  ReactCompat.useRecordApi(
     {
-      ...component,
+      ...ReactCompat.component,
       render: _ =>
         <div> "Helloworld!"->ReasonReact.string </div>
     }
+  )
 }
```

For interface files (`.rei`)

```diff

+[@react.component]
- let make = 'a =>
+ let make = unit =>
-  ReasonReact.component(
-    ReasonReact.stateless,
-    ReasonReact.noRetainedProps,
-    ReasonReact.actionless
-  );
+  React.element;
```

#### Reducer components

For implementation files (`.re`)

```diff
 type action = | Tick;

 type state = {count: int};

-let component = ReasonReact.reducerComponent("MyComponent");

+[@react.component]
- let make = _ => {
+ let make = () => {
+  ReactCompat.useRecordApi(
     {
-      ...component,
+      ...ReactCompat.component,
       /* some lifecycle */
       render: _ =>
         <div> "Helloworld!"->ReasonReact.string </div>
     }
+  )
 }
```

You'll also need to rename:

- `ReasonReact.Update` -> `Update`
- `ReasonReact.UpdateWithSideEffects` -> `UpdateWithSideEffects`
- `ReasonReact.SideEffects` -> `SideEffects`
- `ReasonReact.NoUpdate` -> `NoUpdate`

For interface files (`.rei`)

```diff
-type state;

-type action;

+[@react.component]
- let make = 'a =>
+ let make = unit =>
-  ReasonReact.component(
-    state,
-    ReasonReact.noRetainedProps,
-    action
-  );
+  React.element;
```

## Acknowledgments

Thnks @rickyvetter for the original idea and help through the process
