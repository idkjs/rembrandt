open ElementsTypes;
open Utils;

open! Diff;

type maps = StringMap.t(Dom.domElement);

let setProps = (element: Dom.domElement, attributes: attributes) =>
  attributes
  |> List.map(((key, value)) =>
       if (value === "") {
         Dom.removeAttribute(key, element);
       } else {
         Dom.setAttribute((key, value), element);
       }
     );

let reorderChildren = (element: Dom.domElement, moves: ListDiff.moves) => {
  let staticNodeList = ref(element.childNodes);
  let maps = ref(StringMap.empty);

  moves
  |> List.iter((move: ListDiff.move) =>
       switch (move.moveType, move.item) {
       | (Remove, None) =>
         let childToRemove = Dom.getNthChild(element.children, move.index);
         Dom.removeChild(element, childToRemove) |> ignore;
         staticNodeList := removeFromList(staticNodeList^, move.index);
       | (Insert, Some(item)) =>
         let insertNode =
           if (StringMap.mem(move.index |> string_of_int, maps^)) {
             /* maps[move.item.key].cloneNode(true) */
             StringMap.find(
               move.index |> string_of_int,
               maps^,
             );
           } else {
             Render.render(item);
           };
         staticNodeList :=
           replaceInList(staticNodeList^, move.index, insertNode);
         let _ =
           Dom.insertBefore(
             element,
             insertNode,
             Dom.getNthChild(element.childNodes, move.index),
           );
         ();
       | _ => ()
       }
     );
};

let applyPatches = (element: Dom.domElement, currentPatches: list(patch)) =>
  currentPatches
  |> List.iter(patch =>
       switch (patch.patchType) {
       | Replace =>
         switch (patch.content) {
         | Some(node) =>
           let newDomElement = Render.render(node);
           Dom.replaceChild(
             element->Dom.getParentNode,
             newDomElement,
             element,
           )
           |> ignore;
         | None => ()
         }
       | Props =>
         switch (patch.attributes) {
         | Some(attributes) => setProps(element, attributes) |> ignore
         | None => ()
         }
       | Children => reorderChildren(element, patch.moves) |> ignore
       | Text =>
         switch (patch.content) {
         | Some(node) => Dom.replaceTextNode(element, node.text) |> ignore
         | None => ()
         }
       }
     );

let rec walker = (element: Dom.domElement, patches, step: ref(int)) => {
  let children = Dom.getChildren(element);
  children
  |> List.iter(child => {
       step := step^ + 1;
       walker(child, patches, step);
     });

  if (IntMap.mem(step^, patches)) {
    let currentPatches = IntMap.find(step^, patches);
    applyPatches(element, currentPatches);
  };
};

let patch = (node, patches) => walker(node, patches, ref(0));
