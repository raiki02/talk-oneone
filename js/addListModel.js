function addChatMessage(listModel,text,role="User"){
    listModel.append({"context":text,"role":role})
}

// Append text to the last message in the model (for streaming)
function appendToLast(listModel, text, role="Assistant") {
    if (listModel.count === 0) {
        // No messages yet, create a new one
        listModel.append({"context": text, "role": role})
        return
    }
    var lastIndex = listModel.count - 1
    var last = listModel.get(lastIndex)
    if (last.role === role) {
        // Update in place: remove and re-add with appended text
        var newContext = last.context + text
        listModel.set(lastIndex, {"context": newContext, "role": role})
    } else {
        // Last message is from different role, create new
        listModel.append({"context": text, "role": role})
    }
}
