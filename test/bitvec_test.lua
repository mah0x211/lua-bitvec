local testcase = require('testcase')
local assert = require('assert')
local bitvec = require('bitvec')

function testcase.new_and_tostring()
    local b = bitvec.new()
    assert.is_userdata(b)
    assert.match(tostring(b), 'bitvec: 0x', false)
end

function testcase.nbit_default()
    local b = bitvec.new()
    -- default capacity is BV_BIT (64)
    assert.equal(b:nbit(), 64)
end

function testcase.set_get_unset()
    local b = bitvec.new()
    -- default value is false
    assert.is_false(b:get(0))
    assert.is_false(b:get(63))

    -- set then get returns true
    assert.is_true(b:set(0))
    assert.is_true(b:get(0))

    assert.is_true(b:set(63))
    assert.is_true(b:get(63))

    -- unset clears the bit
    b:unset(0)
    assert.is_false(b:get(0))
end

function testcase.set_auto_resize()
    local b = bitvec.new()
    -- setting beyond default 64 bits triggers resize
    assert.is_true(b:set(128))
    assert.is_true(b:get(128))
    assert.greater(b:nbit(), 64)
end

function testcase.set_negative_rejects()
    local b = bitvec.new()
    local ok, err = pcall(function()
        b:set(-1)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)
end

function testcase.unset_negative_rejects()
    local b = bitvec.new()
    local ok, err = pcall(function()
        b:unset(-1)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)
end

function testcase.get_negative_rejects()
    local b = bitvec.new()
    local ok, err = pcall(function()
        b:get(-1)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)
end

function testcase.setrange_unsetrange()
    local b = bitvec.new()
    assert.is_true(b:setrange(2, 10))
    for i = 2, 10 do
        assert.is_true(b:get(i), 'bit ' .. i .. ' should be set')
    end
    assert.is_false(b:get(1))
    assert.is_false(b:get(11))

    b:unsetrange(2, 10)
    for i = 2, 10 do
        assert.is_false(b:get(i), 'bit ' .. i .. ' should be unset')
    end
end

function testcase.setrange_auto_resize()
    local b = bitvec.new()
    -- setrange beyond default 64 triggers resize
    assert.is_true(b:setrange(60, 200))
    for i = 60, 200 do
        assert.is_true(b:get(i))
    end
end

function testcase.setrange_negative_rejects()
    local b = bitvec.new()
    local ok, err = pcall(function()
        b:setrange(-1, 5)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)

    ok, err = pcall(function()
        b:setrange(0, -1)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)
end

function testcase.unsetrange_negative_rejects()
    local b = bitvec.new()
    local ok, err = pcall(function()
        b:unsetrange(-1, 5)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)

    ok, err = pcall(function()
        b:unsetrange(0, -1)
    end)
    assert.is_false(ok)
    assert.match(err, 'unsigned int expected', false)
end

function testcase.clear()
    local b = bitvec.new()
    b:setrange(0, 63)
    for i = 0, 63 do
        assert.is_true(b:get(i))
    end
    b:clear()
    for i = 0, 63 do
        assert.is_false(b:get(i))
    end
end

function testcase.ffz_empty()
    local b = bitvec.new()
    -- empty bitvec: first zero is at position 0
    assert.equal(b:ffz(), 0)
end

function testcase.ffz_after_set()
    local b = bitvec.new()
    b:set(0)
    assert.equal(b:ffz(), 1)
    b:set(1)
    assert.equal(b:ffz(), 2)
    b:setrange(2, 10)
    assert.equal(b:ffz(), 11)
end

function testcase.ntz_empty()
    local b = bitvec.new()
    -- empty bitvec: number of trailing zeros equals nbit
    assert.equal(b:ntz(), 64)
end

function testcase.ntz_after_set()
    local b = bitvec.new()
    b:set(5)
    assert.equal(b:ntz(), 5)
    b:set(0)
    assert.equal(b:ntz(), 0)
end

function testcase.gc()
    local b = bitvec.new()
    b:set(0)
    b = nil -- luacheck: ignore 311
    collectgarbage('collect')
    collectgarbage('collect')
end
