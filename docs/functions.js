// Highlight the currently focused heading (based on location.hash)
var hash;
function updateHash()
{
    var id = location.hash.substr(1);
    // Remove "hash" from the old element's className
    var oldEl = document.getElementById(hash);
    if (oldEl)
    {
        var className = oldEl.className;
        if (className.length > 4 &&
            className.substr(className.length - 5) == " hash")
        {
            oldEl.className = className.substr(0, className.length - 5);
        }
        else if (className == "hash")
            oldEl.className = "";
        // Remove the top-link
        var top = document.getElementById("top-link");
        if (top)
            oldEl.removeChild(top);
    }
    // Add "hash" to the new element's className
    var newEl = document.getElementById(id)
    // Don't add things to footnote links
    if (newEl)
    {
        if (newEl.className.length != 0)
            newEl.className = newEl.className = " hash";
        else
            newEl.className = "hash";
        // Add a link to the top (unless it's a footnote)
        if (id.substr(0, 6) != "fnref:" && id.substr(0,3) != "fn:")
        {
            var a = document.createElement("a");
            a.href = "#";
            a.title = "Top";
            a.id = "top-link";
            a.onclick = function() { location='#'; updateHash(); };
            a.appendChild(document.createTextNode("^"))
            newEl.insertBefore(a, newEl.firstChild)
        }
    }

    // If there is a nav div, also highlight the hash in the navigation menu
    if (document.getElementById('nav'))
    {
        var base = String(location);
        var end = base.lastIndexOf('#');
        if (end == -1)
            base = base.substring(base.lastIndexOf('/') + 1);
        else
            base = base.substring(base.lastIndexOf('/') + 1, end);
        base = "nav_" + base.replace('.', '_');
        var oldEl = document.getElementById(hash ? base + "_" + hash : base);
        if (oldEl) {
            oldEl = oldEl.getElementsByTagName('a')[0];
            var className = oldEl.className;
            if (className.length > 4 &&
                className.substr(className.length - 5) == " hash")
            {
                oldEl.className = className.substr(0, className.length - 5);
            }
            else if (className == "hash")
                oldEl.className = "";
        }
        var newEl = document.getElementById(id.length > 0 ? base + "_" + id : base);
        if (newEl) {
            newEl = newEl.getElementsByTagName('a')[0];
            if (newEl.className.length != 0)
                newEl.className = newEl.className = " hash";
            else
                newEl.className = "hash";
            // Expand newEl
            newEl = newEl.parentNode.parentNode;
            while (newEl.id != 'nav') {
                if (newEl.tagName.toLowerCase() == 'li')
                    expandMenu(newEl);
                newEl = newEl.parentNode;
            }
        }
    }
    hash = id;
}

// Navigation menu stuff
function expandMenu(li) {
    var div = li.getElementsByTagName('div')[0];
    var span = div.getElementsByTagName('span')[0];
    var ul = li.getElementsByTagName('ul')[0];
    if (! ul)
        return;
    ul.style.display = 'block';
    div.className = 'expanded';
    if (span.textContent)
        span.textContent = '-';
    else
        span.innerText = '-';
}

function collapseMenu(li) {
    var div = li.getElementsByTagName('div')[0];
    var span = div.getElementsByTagName('span')[0];
    var ul = li.getElementsByTagName('ul')[0];
    if (! ul)
        return;
    ul.style.display = 'none';
    div.className = 'collapsed';
    if (span.textContent)
        span.textContent = '+';
    else
        span.innerText = '+';
}

function toggleMenu(li) {
    if (li.getElementsByTagName('div')[0].className == 'collapsed')
        expandMenu(li);
    else
        collapseMenu(li);
}
