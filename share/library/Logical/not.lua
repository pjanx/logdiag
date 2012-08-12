-- Symbol name
local names =
{
	en = "NOT",
	cs = "NOT",
	sk = "NOT",
	pl = "NOT",
	de = "NICHT"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-4, -2, 4, 2}

-- Terminal points
local terminals = {{-4, 0}, {4, 0}}

-- Rendering
local render = function (cr)
	-- The triangle
	cr:move_to (-2, -2)
	cr:line_to (2, 0)
	cr:line_to (-2, 2)
	cr:close_path ()

	-- The circle
	cr:new_sub_path ()
	cr:arc (2.25, 0, 0.25, 0, 2 * math.pi)

	-- The terminals
	cr:move_to (-4, 0)
	cr:line_to (-2, 0)

	cr:move_to (2.5, 0)
	cr:line_to (4, 0)

	cr:stroke ()
end

-- Register the symbol
logdiag.register ("NOT", names, area, terminals, render)


