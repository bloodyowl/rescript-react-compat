type component('state, 'initialState, 'action) = {
  willReceiveProps: self('state, 'action) => 'state,
  willUnmount: self('state, 'action) => unit,
  didUpdate: oldNewSelf('state, 'action) => unit,
  shouldUpdate: oldNewSelf('state, 'action) => bool,
  willUpdate: oldNewSelf('state, 'action) => unit,
  didMount: self('state, 'action) => unit,
  initialState: unit => 'initialState,
  reducer: ('action, 'state) => update('state, 'action),
  render: self('state, 'action) => React.element,
}
and update('state, 'action) =
  | NoUpdate
  | Update('state)
  | SideEffects(self('state, 'action) => unit)
  | UpdateWithSideEffects('state, self('state, 'action) => unit)
and self('state, 'action) = {
  handle:
    'payload.
    (('payload, self('state, 'action)) => unit, 'payload) => unit,

  state: 'state,
  send: 'action => unit,
  onUnmount: (unit => unit) => unit,
}
and oldNewSelf('state, 'action) = {
  oldSelf: self('state, 'action),
  newSelf: self('state, 'action),
};

/** This is not exposed, only used internally so that useReducer can
    return side-effects to run later.
  */
type fullState('state, 'action) = {
  sideEffects: ref(array(self('state, 'action) => unit)),
  state: ref('state),
};

let useRecordApi = componentSpec => {
  open React.Ref;

  let initialState = React.useMemo0(componentSpec.initialState);
  let unmountSideEffects = React.useRef([||]);

  let ({state, sideEffects}, send) =
    React.useReducer(
      (fullState, action) =>
        /**
          Keep fullState.state in a ref so that willReceiveProps can alter it.
          It's the only place we let it be altered.
          Keep fullState.sideEffects so that they can be cleaned without a state update.
          It's important that the reducer only **creates new refs** an doesn't alter them,
          otherwise React wouldn't be able to rollback state in concurrent mode.
         */
        (
          switch (componentSpec.reducer(action, fullState.state^)) {
          | NoUpdate => fullState /* useReducer returns of the same value will not rerender the component */
          | Update(state) => {...fullState, state: ref(state)}
          | SideEffects(sideEffect) => {
              ...fullState,
              sideEffects:
                ref(
                  Js.Array.concat(fullState.sideEffects^, [|sideEffect|]),
                ),
            }
          | UpdateWithSideEffects(state, sideEffect) => {
              sideEffects:
                ref(
                  Js.Array.concat(fullState.sideEffects^, [|sideEffect|]),
                ),
              state: ref(state),
            }
          }
        ),
      {sideEffects: ref([||]), state: ref(initialState)},
    );

  /** This is the temp self for willReceiveProps */
  let rec self = {
    handle: (fn, payload) => fn(payload, self),
    send,
    state: state^,
    onUnmount: sideEffect =>
      Js.Array.push(sideEffect, unmountSideEffects->current)->ignore,
  };

  let upToDateSelf = React.useRef(self);

  let hasBeenCalled = React.useRef(false);

  /** There might be some potential issues with willReceiveProps,
      treat it as it if was getDerivedStateFromProps. */
  state :=
    componentSpec.willReceiveProps(self);

  let self = {
    handle: (fn, payload) => fn(payload, upToDateSelf->current),
    send,
    state: state^,
    onUnmount: sideEffect =>
      Js.Array.push(sideEffect, unmountSideEffects->current)->ignore,
  };

  let oldSelf = React.useRef(self);

  let _mountUnmountEffect =
    React.useEffect0(() => {
      componentSpec.didMount(self);
      Some(
        () => {
          Js.Array.forEach(fn => fn(), unmountSideEffects->current);
          /* shouldn't be needed but like - better safe than sorry? */
          unmountSideEffects->setCurrent([||]);
          componentSpec.willUnmount(upToDateSelf->React.Ref.current);
        },
      );
    });

  let _didUpdateEffect =
    React.useEffect(() => {
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
        let sideEffectsToRun = Js.Array.sliceFrom(0, sideEffects^);
        sideEffects := [||];
        Js.Array.forEach(func => func(self), sideEffectsToRun);
      };
      None;
    },
    [|sideEffects|],
  );

  let mostRecentAllowedRender =
    React.useRef(React.useMemo0(() => componentSpec.render(self)));

  upToDateSelf->setCurrent(self);

  if (hasBeenCalled->current
      && componentSpec.shouldUpdate({
           oldSelf: oldSelf->current,
           newSelf: self,
         })) {
    componentSpec.willUpdate({oldSelf: oldSelf->current, newSelf: self});
    mostRecentAllowedRender->setCurrent(componentSpec.render(self));
  };
  mostRecentAllowedRender->current;
};

module Defaults = {
  let anyToUnit = _ => ();
  let anyToTrue = _ => true;
  let willReceivePropsDefault: self('state, 'action) => 'state =
    ({state}) => state;
  let renderDefault = _self => React.string("RenderNotImplemented");
  let initialStateDefault = () => ();
  let reducerDefault: ('action, 'state) => update('state, 'action) =
    (_action, _state) => NoUpdate;
};

let component: component('state, 'initialState, 'action) = {
  didMount: Defaults.anyToUnit,
  willReceiveProps: Defaults.willReceivePropsDefault,
  didUpdate: Defaults.anyToUnit,
  willUnmount: Defaults.anyToUnit,
  willUpdate: Defaults.anyToUnit,
  shouldUpdate: Defaults.anyToTrue,
  render: Defaults.renderDefault,
  initialState: Defaults.initialStateDefault,
  reducer: Defaults.reducerDefault,
};

let useMount = func => {
  React.useEffect0(() => {
    func();
    None;
  });
};
