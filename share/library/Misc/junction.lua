-- Symbol name
local names =
{
	en = "Junction",
	cs = "Spoj",
	sk = "Spoj",
	pl = "Złącze",
	de = "Anschluss"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-0.5, -0.5, 0.5, 0.5}

-- Terminal points
local terminals = {}

-- Rendering
local render = function (cr)
	-- The disk
	cr:arc (0, 0, 0.3, 0, math.pi * 2)
	cr:fill ()
end

-- Register the symbol
logdiag.register ("Junction", names, area, terminals, render)


