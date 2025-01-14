Modules.require("./UI_SketchOwnerInfo.css");
open Utils;

[@react.component]
let make = (~noteLastEdited=?, ~className=?, ~owner, ~noteCompilerVersion) => {
  <div className={Cn.make(["UI_SketchOwnerInfo", Cn.unwrap(className)])}>
    <Router.Link route={Route.User(owner##username)}>
      {owner##avatar
       =>> (
         avatar =>
           <img
             src=avatar
             width="24"
             height="24"
             className="UI_SketchOwnerInfo__avatar"
             alt={owner##username ++ " avatar"}
           />
       )}
      <span className="UI_SketchOwnerInfo__username">
        {owner##username->str}
      </span>
    </Router.Link>
    {noteLastEdited
     =>> (
       noteLastEdited =>
         <>
           <span className="UI_SketchOwnerInfo__lastEdited">
             "last edited"->str
             <UI_DateTime
               date=noteLastEdited
               className="UI_SketchOwnerInfo__time"
             />
           </span>
           <span className="UI_SketchOwnerInfo__lastEdited">
             {("OCaml " ++ noteCompilerVersion->CompilerVersion.toUiString)
              ->str}
           </span>
         </>
     )}
  </div>;
};
