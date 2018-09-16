.pragma library

function pushToStack(stack, page) {
    if(page && stack.currentItem !== page) {
        if(stack.depth === 1) {
            stack.replace(page)
        } else {
            stack.pop(null)
            stack.replace(page)
        }
    }
}
