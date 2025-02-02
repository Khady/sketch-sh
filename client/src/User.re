Modules.require("./User.css");
open Utils;

module GetNotes = [%graphql
  {|
  query getNotes($userName: String!, $limit: Int, $offset: Int) {
    note(
      where: {user: {username: {_eq: $userName}}}
      order_by: [{
        updated_at: desc
      }],
      limit: $limit,
      offset: $offset
      ) {
        id
        title
        date: updated_at
        compilerVersion: compiler_version
      }
    }
    |}
];

module GetNotesComponent = ReasonApollo.CreateQuery(GetNotes);

let getNotesQuery = (~userName, ~offset=0, ()) =>
  GetNotes.make(~userName, ~offset, ~limit=Config.sketchListLimit, ());

external unsafeFromJson: Js.Json.t => GetNotes.t = "%identity";
external unsafeToJson: GetNotes.t => Js.Json.t = "%identity";

let updateQuery = (prev, next) => {
  let prev = unsafeFromJson(prev);

  (
    switch (next.ReasonApolloQuery.fetchMoreResult) {
    | None => prev
    | Some(moreNotes) =>
      let moreNotes = unsafeFromJson(moreNotes);
      let newNotes = {
        "note": Belt.Array.concat(prev##note, moreNotes##note),
      };
      Utils.Object.merge(prev, newNotes);
    }
  )
  |> unsafeToJson;
};

let shouldFetchMore = (note, count) =>
  Array.length(note) >= Config.sketchListLimit * count;

type state = {count: int};
type action =
  | ChangeCount;

let initialState = () => {count: 1};

let reducer = (action, state) =>
  switch (action) {
  | ChangeCount => ReactCompat.Update({count: 1 + state.count})
  };

[@react.component]
let make = (~userName) =>
  ReactCompat.useRecordApi({
    ...ReactCompat.component,
    initialState,
    reducer,
    render: self => {
      let notesQuery = getNotesQuery(~userName, ());
      <section className="Layout__center User">
        <h1> {j|$(userName)'s sketches|j}->str </h1>
        <GetNotesComponent
          fetchPolicy="network-only" variables=notesQuery##variables>
          ...{({result, fetchMore}) =>
            switch (result) {
            | Loading =>
              <div style={ReactDOMRe.Style.make(~width="500px", ())}>
                <UI_SketchList.Placeholder />
              </div>
            | Error(error) => error.message->str
            | Data(response) =>
              let notesQuery =
                getNotesQuery(
                  ~userName,
                  ~offset=Array.length(response##note),
                  (),
                );
              let fetchMore = _e =>
                Js.Promise.(
                  fetchMore(
                    ~variables=Some(notesQuery##variables),
                    ~updateQuery,
                    (),
                  )
                  |> then_(_ => {
                       self.send(ChangeCount);
                       resolve();
                     })
                )
                |> ignore;

              shouldFetchMore(response##note, self.state.count)
                ? <UI_SketchList
                    sketches=response##note
                    noSketches={<UI_NoSketches />}
                    fetchMore
                  />
                : <UI_SketchList
                    sketches=response##note
                    noSketches={<UI_NoSketches />}
                  />;
            }
          }
        </GetNotesComponent>
      </section>;
    },
  });
