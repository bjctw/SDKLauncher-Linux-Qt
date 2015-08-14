

/////////////////////////
//SmilNode

ReadiumSDK.Models.Smil.SmilNode = function() {

    this.parent = null;

};

ReadiumSDK.Models.Smil.TimeContainerNode = function() {
    this.id = "";
    this.epubtype = "";
    this.children = undefined;
};

ReadiumSDK.Models.Smil.TimeContainerNode.prototype = new ReadiumSDK.Models.Smil.SmilNode();

//////////////////////////
//MediaNode

ReadiumSDK.Models.Smil.MediaNode = function() {
    this.src = "";
};

ReadiumSDK.Models.Smil.MediaNode.prototype = new ReadiumSDK.Models.Smil.SmilNode();

////////////////////////////

ReadiumSDK.Models.Smil.SeqNode = function() {
    this.children = [];
    this.nodeType = "seq";
    this.textref = "";

};

ReadiumSDK.Models.Smil.SeqNode.prototype = new ReadiumSDK.Models.Smil.TimeContainerNode();

//////////////////////////
//SeqNode

ReadiumSDK.Models.Smil.ParNode = function() {
    this.children = [];
    this.nodeType = "par";
};

ReadiumSDK.Models.Smil.ParNode.prototype = new ReadiumSDK.Models.Smil.TimeContainerNode();

//////////////////////////
//TextNode

ReadiumSDK.Models.Smil.TextNode = function() {

    this.nodeType = "text";
};

ReadiumSDK.Models.Smil.TextNode.prototype = new ReadiumSDK.Models.Smil.MediaNode();

///////////////////////////
//AudioNode

ReadiumSDK.Models.Smil.AudioNode = function() {

    this.nodeType = "audio";
    this.clipBegin = "";
    this.clipEnd = "";

};
ReadiumSDK.Models.Smil.AudioNode.prototype = new ReadiumSDK.Models.Smil.MediaNode();

//////////////////////////////
//SmilModel

ReadiumSDK.Models.SmilModel = function() {

    this.children = []; //collection of seq or par smil nodes
    this.id = undefined; //manifest item id
    this.href = undefined; //href of the .smil source file
    this.duration = undefined;

};

ReadiumSDK.Models.SmilModel.fromSmilDTO = function(smilDTO) {

    var smilModel = new ReadiumSDK.Models.SmilModel();
    smilModel.id = smilDTO.id;
    smilModel.href = smilDTO.href;
    smilModel.smilVersion = smilDTO.smilVersion;
    smilModel.duration = smilDTO.duration;

    var safeCopyProperty = function(property, from, to, isRequired) {

        if(property in from) {

            if( !(property in to) ) {
                console.debug("property " + property + " not declared in smil node " + to.nodeType);
            }

            to[property] = from[property];
        }
        else if(isRequired) {
            console.error("Required property " + property + " not found in smil node " + from.nodeType);
        }
    };

    var createNodeFromDTO = function(nodeDTO) {

        var node;

        if(nodeDTO.nodeType == "seq") {
            node = new ReadiumSDK.Models.Smil.SeqNode();
            safeCopyProperty("textref", nodeDTO, node, true);
            safeCopyProperty("id", nodeDTO, node);
            safeCopyProperty("epubtype", nodeDTO, node);

            copyChildren(nodeDTO, node);
        }
        else if (nodeDTO.nodeType == "par") {
            node = new ReadiumSDK.Models.Smil.ParNode();
            safeCopyProperty("id", nodeDTO, node);
            safeCopyProperty("epubtype", nodeDTO, node);

            copyChildren(nodeDTO, node);
        }
        else if (nodeDTO.nodeType == "text") {
            node = new ReadiumSDK.Models.Smil.TextNode();
            safeCopyProperty("src", nodeDTO, node, true);
            safeCopyProperty("id", nodeDTO, node);
        }
        else if (nodeDTO.nodeType == "audio") {
            node = new ReadiumSDK.Models.Smil.AudioNode();
            safeCopyProperty("src", nodeDTO, node, true);
            safeCopyProperty("id", nodeDTO, node);
            safeCopyProperty("clipBegin", nodeDTO, node);
            safeCopyProperty("clipEnd", nodeDTO, node);
        }
        else {
            console.error("Unexpected smil node type: " + nodeDTO.nodeType);
            return undefined;
        }

        return node;

    };

    var copyChildren = function(from, to) {

        var count = from.children.length;

        for(var i = 0; i < count; i++) {
            var node = createNodeFromDTO(from.children[i]);
            node.parent = to;
            to.children.push(node);
        }

    };

    copyChildren(smilDTO, smilModel);

    return smilModel;

};
