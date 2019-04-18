/* For now, we'll use the existing building blocks in ReasonReact,
   but we might internalize them in the future. */
open ReasonReact;

/** This is not exposed, only used internally so that useReducer can
    return side-effects to run later.
  */
type fullState('state, 'retainedProps, 'action) = {
  sideEffects: ref(array(self('state, 'retainedProps, 'action) => unit)),
  state: ref('state),
};

let useRecordApi = componentSpec => {
  open React.Ref;

  let initialState = React.useMemo0(componentSpec.initialState);
  let unmountSideEffects = React.useRef([||]);

  let ({state, sideEffects}, send) =
    React.useReducer(
      (fullState, action) => {
        /**
          Keep fullState.state in a ref so that willReceiveProps can alter it.
          It's the only place we let it be altered.
          Keep fullState.sideEffects so that they can be cleaned without a state update.
          It's important that the reducer only **creates new refs** an doesn't alter them,
          otherwise React wouldn't be able to rollback state in concurrent mode.
         */
        switch (componentSpec.reducer(action, fullState.state^)) {
        | NoUpdate => fullState /* useReducer returns of the same value will not rerender the component */
        | Update(state) => {...fullState, state: ref(state)}
        | SideEffects(sideEffect) => {
            ...fullState,
            sideEffects:
              ref(Js.Array.concat(fullState.sideEffects^, [|sideEffect|])),
          }
        | UpdateWithSideEffects(state, sideEffect) => {
            sideEffects:
              ref(Js.Array.concat(fullState.sideEffects^, [|sideEffect|])),
            state: ref(state),
          }
        };
      },
      {sideEffects: ref([||]), state: ref(initialState)},
    );

  /** This is the temp self for willReceiveProps */ 
  let rec self = {
    handle: (fn, payload) => fn(payload, self),
    retainedProps: componentSpec.retainedProps,
    send,
    state: state^,
    onUnmount: sideEffect =>
      Js.Array.push(sideEffect, unmountSideEffects->current)->ignore,
  };

  let hasBeenCalled = React.useRef(false);

  /** There might be some potential issues with willReceiveProps,
      treat it as it if was getDerivedStateFromProps. */
  state := componentSpec.willReceiveProps(self);

  let rec self = {
    handle: (fn, payload) => fn(payload, self),
    retainedProps: componentSpec.retainedProps,
    send,
    state: state^,
    onUnmount: sideEffect =>
      Js.Array.push(sideEffect, unmountSideEffects->current)->ignore,
  };

  let oldSelf = React.useRef(self);

  let _mountUnmountEffect =
    React.useLayoutEffect0(() => {
      componentSpec.didMount(self);
      Some(
        () => {
          Js.Array.forEach(fn => fn(), unmountSideEffects->current);
          /* shouldn't be needed but like - better safe than sorry? */
          unmountSideEffects->setCurrent([||]);
          componentSpec.willUnmount(self);
        },
      );
    });

  let _didUpdateEffect =
    React.useLayoutEffect(() => {
      if (hasBeenCalled->current) {
        componentSpec.didUpdate({oldSelf: oldSelf->current, newSelf: self});
      } else {
        hasBeenCalled->setCurrent(true);
      };
      oldSelf->setCurrent(self);
      None;
    });

  /** Because sideEffects are only added through a **new** ref, 
      we can use the ref itself as the dependency. This way the
      effect doesn't re-run after a cleanup.
  */
  React.useEffect1(
    () => {
      if (Js.Array.length(sideEffects^) > 0) {
        Js.Array.forEach(func => func(self), sideEffects^);
        sideEffects := [||];
      };
      None;
    },
    [|sideEffects|],
  );

  let mostRecentAllowedRender =
    React.useRef(React.useMemo0(() => componentSpec.render(self)));

  if (hasBeenCalled->current
      && oldSelf->current.state !== self.state
      && componentSpec.shouldUpdate({
           oldSelf: oldSelf->current,
           newSelf: self,
         })) {
    componentSpec.willUpdate({oldSelf: oldSelf->current, newSelf: self});
    mostRecentAllowedRender->setCurrent(componentSpec.render(self));
  };
  mostRecentAllowedRender->current;
};
