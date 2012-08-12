-- Symbol name
local names =
{
	en = "Ground",
	cs = "Zem",
	sk = "Uzemnenie",
	pl = "Ziemia",
	de = "Masse"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-1, -1, 1, 2}

-- Terminal points
local terminals = {{0, -1}}

-- Rendering
local render = function (cr)
	-- The vertical line
	cr:move_to (0, -1)
	cr:line_to (0, 0.5)

	-- The horizontal lines
	cr:move_to (-1, 0.5)
	cr:line_to (1, 0.5)

	cr:move_to (-0.75, 1.1)
	cr:line_to (0.75, 1.1)

	cr:move_to (-0.5, 1.7)
	cr:line_to (0.5, 1.7)

	cr:stroke ()
end

-- Register the symbol
logdiag.register ("Ground", names, area, terminals, render)


