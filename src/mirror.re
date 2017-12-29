[%bs.raw {|require('./mirror.css')|}];

[@bs.module] external logo : string = "./logo.svg";

[@bs.val] [@bs.scope "Math"] external random : unit => float = "random";

[@bs.val] [@bs.scope "Math"] external floor : float => int = "floor";

type action =
  | Tick
  | LoadedQuotes(list(string), int);

type state = {
  date: Js.Date.t,
  timerId: ref(option(Js.Global.intervalId)),
  quotes: list(string),
  quote: string
};

let component = ReasonReact.reducerComponent("Mirror");

let str = ReasonReact.stringToElement;

let fetchQuotes = (callback) =>
  Js.Promise.(
    Axios.get("./quotes.json")
    |> then_(
         (response) => {
           callback(Array.to_list(response##data##quotes));
           resolve()
         }
       )
    |> ignore
  );

let make = (_children) => {
  ...component,
  initialState: () => {date: Js.Date.make(), timerId: ref(None), quotes: [], quote: ""},
  reducer: (action, state) =>
    switch action {
    | Tick => ReasonReact.Update({...state, date: Js.Date.make()})
    | LoadedQuotes(fetchedQuotes, randomNum) =>
      ReasonReact.Update({
        ...state,
        quotes: fetchedQuotes,
        quote: List.nth(fetchedQuotes, randomNum)
      })
    },
  didMount: (self) => {
    self.state.timerId := Some(Js.Global.setInterval(self.reduce((_) => Tick), 1000));
    fetchQuotes(
      self.reduce(
        (quotes) => {
          let max = float_of_int(List.length(quotes));
          let getRandomNumber = floor(random() *. max);
          LoadedQuotes(quotes, getRandomNumber)
        }
      )
    );
    ReasonReact.NoUpdate
  },
  willUnmount: ({state}) =>
    switch state.timerId^ {
    | Some(id) => Js.Global.clearInterval(id)
    | _ => ()
    },
  render: ({state}) =>
    <div className="Mirror" style=(ReactDOMRe.Style.make(~color="#fff", ~fontSize="48px", ()))>
      <div className="Mirror_Top">
        <div> (str(Js.Date.toLocaleTimeString(state.date))) </div>
        <div> (str(Js.Date.toDateString(state.date))) </div>
      </div>
      <div className="Mirror_Bottom"> <div> (str(state.quote)) </div> </div>
    </div>
};