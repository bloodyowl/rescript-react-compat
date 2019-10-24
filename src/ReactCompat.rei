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

let useRecordApi: component('state, 'state, 'action) => React.element;

let component: component('state, unit, 'action);

let useMount: (unit => unit) => unit;
