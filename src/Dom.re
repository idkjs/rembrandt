type domElementValue =
  | String
  | Int;
// type domElement = {
//   children: list(Webapi.Dom.Element.t),
//   childNodes: list(Webapi.Dom.Element.t),
//   parentElement: Webapi.Dom.Element.t,
//   nodeType: int,
//   getAttribute: string => string,
//   value: string,
// };
type domElement = {
  children: list(domElement),
  childNodes: list(domElement),
  parentElement: domElement,
  nodeType: int,
  getAttribute: string => string,
  value: string,
};

type event = {
  target: domElement,
  preventDefault: unit => unit,
};

type eventHandler = event => unit;

[@bs.val]
external createElement: string => domElement = "document.createElement";

let createShadowElement: string => domElement = [%bs.raw
  {|
    function(mode) {
      var element = document.createElement('div');
      element.attachShadow({ mode: mode });

      return element;
    }
  |}
];

let _getChildrenArray: domElement => array(domElement) = [%bs.raw
  {|
    function (element) {
      if (element.shadowRoot) {
        return element.shadowRoot.childNodes;
      }
      return element.childNodes;
    }
  |}
];

let getChildren = (element: domElement): list(domElement) =>
  _getChildrenArray(element)->Array.to_list;

let getNthChild: (list(domElement), int) => domElement = [%bs.raw
  {|
  function (children, index) {
      return chilren[index];
    }
  |}
];

let removeAttribute: (string, domElement) => domElement = [%bs.raw
  {|
  function (key, element) {
      element.removeAttribute(key);
      return element;
    }
  |}
];

let setAttribute: ((string, string), domElement) => domElement = [%bs.raw
  {|
 function (attribute, element) {
    element.setAttribute(attribute[0], attribute[1]);
    return element;
  }
|}
];

let _setAttributes: ((string, string), domElement) => domElement = [%bs.raw
  {|
 function (attribute, element) {
    element.setAttribute(attribute[0], attribute[1]);
    return element;
  }
|}
];

let setAttributes =
    (attributes: list((string, string)), domElement: domElement) => {
  let _ = attributes |> List.map(attribute => _setAttributes(attribute, domElement));
  domElement;
};

let setPosition: (int, domElement) => domElement = [%bs.raw
  {|
    function (position, element) {
      element.position = position;
      return element;
    }
|}
];

let replaceChild: (domElement, domElement, domElement) => domElement = [%bs.raw
  {|
      function(parent, newNode, oldNode) {
        if (!parent.shadowRoot) {
          return parent.shadowRoot.replaceChild(newNode, oldNode);
        }
        return parent.replaceChild(newNode, oldNode);
      }
  |}
];

let replaceTextNode: (domElement, string) => domElement = [%bs.raw
  {|
    function(element, text) {
      if (element.textContent) {
        element.textContent = text
      } else {
        element.nodeValue = text
      }
      return element;
    }
  |}
];

let init: (string, domElement) => domElement = [%bs.raw
  {|
    function(id, element) {
      return document.getElementById(id).appendChild(element);
    }
  |}
];

let update: (string, domElement) => domElement = [%bs.raw
  {|
    function(id, element) {
      var app = document.getElementById(id);
      return app.replaceChild(element, app.firstChild);
    }
  |}
];

let createTextNode: string => domElement = [%bs.raw
  {|
    function(text) {
      return document.createTextNode(text);
    }
|}
];

let getParentNode: domElement => domElement = [%bs.raw
  {|
    function(element) {
      return element.parentNode;
    }
  |}
];

let removeChild: (domElement, domElement) => domElement = [%bs.raw
  {|
    function(parent, child) {
      return parent.removeChild(child);
    }
  |}
];

let appendChildToShadowRoot: (list(domElement), domElement) => domElement = [%bs.raw
  {|
  function (children, parent) {
      for (let i=0; i < children.length; i++) {
        if (Array.isArray(children[i])) {
          appendChildToShadowRoot(children[i], parent);
        } else if (typeof children[i] !== 'number') {
          parent.shadowRoot.appendChild(children[i]);
        }
      }
      return parent;
   }
  |}
];

let appendChild: (list(domElement), domElement) => domElement = [%bs.raw
  {|
   function (children, parent) {
      for (let i=0; i < children.length; i++) {
        if (Array.isArray(children[i])) {
          appendChild(children[i], parent);
        } else if (typeof children[i] !== 'number') {
          parent.appendChild(children[i]);
        }
      }
      return parent;
   }
  |}
];

let addEventListener: (eventHandler, string, domElement) => domElement = [%bs.raw
  {|
    function(handler, eventName, parent) {
      return parent.addEventListener(eventName, function(e) {
        handler(e);
      });
    }
  |}
];

let insertBefore: (domElement, domElement, domElement) => domElement = [%bs.raw
  {|
    function(parent, newElement, referenceElement) {
      parentNode.insertBefore(newElement, referenceElement || null);
      return parent;
    }
  |}
];

let setHandlers =
    (handlers: list((string, option(eventHandler))), parent: domElement)
    : domElement => {
  let _ =handlers
  |> List.map(((name, handler)) =>
       switch (handler) {
       | None => parent
       | Some(h) => addEventListener(h, name, parent)
       }
     );
  parent;
};
